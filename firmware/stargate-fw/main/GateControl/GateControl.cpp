#include "GateControl.hpp"
#include "../Wormhole/Wormhole.hpp"
#include "../FWConfig.hpp"
#include "../HW/HW.hpp"
#include "misc-formula.h"

#define TAG "GateControl"

GateControl::GateControl() :
    m_bIsCancelAction(false)
{
    // Constructor code here
}

void GateControl::Init()
{

}

void GateControl::StartTask()
{
    // Stepper control timer
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &tmr_signal_callback,
        .arg = (Stepper*)&this->m_stepper,
        .dispatch_method = ESP_TIMER_ISR,
        .name = "stepper_timer",
    };

	if (xTaskCreatePinnedToCore(TaskRunning, "GateControl", FWCONFIG_GATECONTROL_STACKSIZE, (void*)this, FWCONFIG_GATECONTROL_PRIORITY_DEFAULT, &m_sGateControlHandle, FWCONFIG_GATECONTROL_COREID) != pdPASS )
	{
		ESP_ERROR_CHECK(ESP_FAIL);
	}

    /* Start the timers */
    this->m_stepper.sTskControlHandle = m_sGateControlHandle;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &this->m_stepper.sSignalTimerHandle));
}

void GateControl::QueueAction(ECmd cmd)
{
    m_eCmd = cmd;
}

void GateControl::AbortAction()
{
    m_bIsCancelAction = true;
}

void GateControl::TaskRunning(void* pArg)
{
    GateControl* gc = (GateControl*)pArg;

    ESP_LOGI(TAG, "Gatecontrol task started and ready.");

    // Get a stargate instance based on parameters
    UniverseGate& universeGate = GateFactory::GetUniverseGate();

    Wormhole wormhole(Wormhole::EType::NormalSG1);

    // Dialing
    while(true)
    {
        const ECmd eCmd = gc->m_eCmd;
        if (eCmd == ECmd::Idle)
        {
            // TODO: Will be replaced by a manual event.
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Reset the receiving ..
        gc->m_bIsCancelAction = false;
        gc->m_eCmd = ECmd::Idle;

        switch(eCmd)
        {
            case ECmd::AutoCalibrate:
            {
                ESP_LOGI(TAG, "Autocalibrate in progress.");
                if (!gc->AutoCalibrate()) {
                    ESP_LOGE(TAG, "Autocalibration failed.");
                }
                else {
                    // Will move it at it's home position, it should go very fast.
                    ESP_LOGI(TAG, "Autocalibrate succeeded.");
                    gc->AutoHome();
                }
                break;
            }
            case ECmd::AutoHome:
            {
                ESP_LOGI(TAG, "Auto-home started.");
                if (!gc->AutoHome()) {
                    ESP_LOGE(TAG, "Auto-home failed.");
                }
                else {
                    ESP_LOGI(TAG, "Auto-home succeeded.");
                }
                break;
            }
            case ECmd::DialAddress:
            {
                if (gc->DialAddress())
                {
                    gc->AutoHome();
                }

                break;
            }
            default:
            case ECmd::Idle:
                break;
        }
    }
}

bool GateControl::AutoCalibrate()
{
    bool bSucceeded = false;
    {
    const uint32_t u32Timeout = 40*1000;

    // We need two transitions from LOW to HIGH.
    // we give it 40s maximum to find the home.
    HW::getI()->PowerUpStepper();
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Finding home in progress");
    if (!SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, nullptr)) {
        ESP_LOGE(TAG, "Timeout condition during auto-calibrate, rotation steps part #1");
        goto ERROR;
    }
    ESP_LOGI(TAG, "Home has been found once");
    int32_t s32NewStepsPerRotation = 0;
    if (!SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, &s32NewStepsPerRotation)) {
        ESP_LOGE(TAG, "Timeout condition during auto-calibrate, rotation steps part #2");
        goto ERROR;
    }

    ESP_LOGI(TAG, "Home has been found a second time, step: %" PRId32, s32NewStepsPerRotation);

    // Find the gap.
    // Continue to move until it get out of the home range.
    int32_t s32Gap = 0;

    if (!SpinUntil(ESpinDirection::CCW, ETransition::Failing, u32Timeout, &s32Gap)) {
        ESP_LOGE(TAG, "Timeout condition during auto-calibrate, deadband part #1");
        goto ERROR;
    }

    if (!SpinUntil(ESpinDirection::CW, ETransition::Rising, u32Timeout, &s32Gap)) {
        ESP_LOGE(TAG, "Timeout condition during auto-calibrate, deadband part #2");
        goto ERROR;
    }

    ESP_LOGI(TAG, "Ticks per rotation: %" PRId32 ", time per rotation, gap: % " PRId32, s32NewStepsPerRotation, s32Gap);

    // Save the calibration result.
    Settings::getI().SetValueInt32(Settings::Entry::StepsPerRotation, false, s32NewStepsPerRotation);
    Settings::getI().SetValueInt32(Settings::Entry::RingHomeGapRange, false, s32Gap);
    Settings::getI().Commit();
    }
    bSucceeded = true;
    goto END;
    ERROR:
    bSucceeded = false;
    END:
    // Go into the other direction until it get out of the sensor
    HW::getI()->PowerDownStepper();
    return bSucceeded;
}

bool GateControl::AutoHome()
{
    bool bSucceeded = false;
    {
    HW::getI()->PowerUpStepper();
    vTaskDelay(pdMS_TO_TICKS(100));

    const int32_t s32NewStepsPerRotation = Settings::getI().GetValueInt32(Settings::Entry::StepsPerRotation);
    const int32_t s32Gap = Settings::getI().GetValueInt32(Settings::Entry::RingHomeGapRange);
    if (s32NewStepsPerRotation == 0 || s32Gap == 0)
    {
        ESP_LOGE(TAG, "Auto-calibration needs to be done.");
        goto ERROR;
    }

    const uint32_t u32Timeout = 40*1000;

    // If the ring is already near the home sensor, we just need to move a little bit.
    if (HW::getI()->GetIsHomeSensorActive()) {
        ESP_LOGI(TAG, "Homing using the fast algorithm");

        if (!SpinUntil(ESpinDirection::CW, ETransition::Failing, u32Timeout, nullptr)) {
            ESP_LOGE(TAG, "Cannot complete the auto-home, part #1");
            goto ERROR;
        }
        if (!SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, nullptr)) {
            ESP_LOGE(TAG, "Cannot complete the auto-home, part #2");
            goto ERROR;
        }
    }
    else {
        ESP_LOGI(TAG, "Homing using the slow algorithm");
        if (!SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, nullptr)) {
            ESP_LOGE(TAG, "Cannot complete the auto-home");
            goto ERROR;
        }
    }

    // Move by half the deadband offset.
    // this is the real 0 position
    const int32_t s32HalfDeadband = s32Gap / 2;
    ESP_LOGI(TAG, "Moving a little bit to take care of the deadband, offset: %" PRId32, s32HalfDeadband);
    for(int i = 0; i < s32HalfDeadband; i++)
    {
        HW::getI()->StepStepperCCW();
        vTaskDelay(1);
    }

    m_s32CurrentPositionTicks = 0;
    m_bIsHomingDone = true;
    bSucceeded = true;
    goto END;
    }
    ERROR:
    bSucceeded = false;
    END:
    // Go into the other direction until it get out of the sensor
    HW::getI()->PowerDownStepper();
    return bSucceeded;
}

bool GateControl::DialAddress()
{
    bool ret = false;
    {
    HW::getI()->PowerUpStepper();
    vTaskDelay(pdMS_TO_TICKS(100));

    if (!m_bIsHomingDone)
    {
        ESP_LOGE(TAG, "Homing need to be done");
        goto ERROR;
    }

    UniverseGate& universeGate = GateFactory::GetUniverseGate();

    AnimRampLight(true);

    const int32_t s32NewStepsPerRotation = Settings::getI().GetValueInt32(Settings::Entry::StepsPerRotation);

    // m_bIsHomingDone / m_s32CurrentPositionTicks
    const Chevron chevrons[] = { Chevron::Chevron1, Chevron::Chevron2, Chevron::Chevron3, Chevron::Chevron4, Chevron::Chevron5, Chevron::Chevron6, Chevron::Chevron7_Master, Chevron::Chevron8, Chevron::Chevron9 };
    //const uint8_t symbols[] = { 5, 10, 20, 30, 3, 16, 1 };
    const uint8_t symbols[] = { 1, 36, 2, 35, 3, 34, 18 };

    for(int32_t i = 0; i < (sizeof(symbols)/sizeof(symbols[0])); i++)
    {
        if (m_bIsCancelAction) {
            ESP_LOGE(TAG, "Unable to complete dialing, cancelled by the user");
            goto ERROR;
        }

        const uint8_t u8Symbol = symbols[i];
        const Chevron currentChevron = chevrons[i];

        // Dial sequence ...
        const int32_t s32LedIndex = universeGate.SymbolToLedIndex(u8Symbol);
        const double dAngle = (universeGate.LEDIndexToDeg(s32LedIndex));
        const int32_t s32SymbolToTicks = (dAngle/360)*s32NewStepsPerRotation;

        const int32_t s32MoveTicks = MISCFA_CircleDiffd32(m_s32CurrentPositionTicks, s32SymbolToTicks, s32NewStepsPerRotation);

        ESP_LOGI(TAG, "led index: %" PRId32 ", angle: %.2f, symbol2Ticks: %" PRId32, s32LedIndex, dAngle, s32SymbolToTicks);
        MoveStepperTo(s32MoveTicks);

        m_s32CurrentPositionTicks = s32SymbolToTicks;
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    ret = true;
    goto END;
    }
    ERROR:
    ret = false;
    END:
    HW::getI()->PowerDownStepper();
    AnimRampLight(false);
    return ret;
}

bool GateControl::SpinUntil(ESpinDirection eSpinDirection, ETransition eTransition, uint32_t u32TimeoutMS, int32_t* ps32refTickCount)
{
    TickType_t ttStart = xTaskGetTickCount();
    bool bOldSensorState = HW::getI()->GetIsHomeSensorActive();

    while ((xTaskGetTickCount() - ttStart) < pdMS_TO_TICKS(40*1000))
    {
        if (m_bIsCancelAction) {
            ESP_LOGE(TAG, "Unable to complete the spin operation, cancelled by the user");
            return false;
        }

        const bool bNewHomeSensorState = HW::getI()->GetIsHomeSensorActive();

        if (eSpinDirection == ESpinDirection::CCW) {
            HW::getI()->StepStepperCCW();
            if (ps32refTickCount != nullptr) {
                (*ps32refTickCount)++;
            }
        }
        if (eSpinDirection == ESpinDirection::CW) {
            HW::getI()->StepStepperCW();
            if (ps32refTickCount != nullptr) {
                (*ps32refTickCount)--;
            }
        }

        const bool bIsRising = !bOldSensorState && bNewHomeSensorState;
        const bool bIsFalling = bOldSensorState && !bNewHomeSensorState;
        if (ETransition::Rising == eTransition && bIsRising)
        {
            ESP_LOGI(TAG, "Rising transistion detected");
            return true;
        }
        else if (ETransition::Failing == eTransition && bIsFalling) {
            ESP_LOGI(TAG, "Failing transistion detected");
            return true;
        }

        bOldSensorState = bNewHomeSensorState;
        vTaskDelay(1);
    }

    ESP_LOGE(TAG, "Unable to complete the spin operation");
    return false;
}

bool GateControl::MoveStepperTo(int32_t s32Ticks)
{
    // Setup the parameters
    this->m_stepper.s32Count = 0;
    this->m_stepper.s32Target = abs(s32Ticks);
    this->m_stepper.s32Period = 1;
    this->m_stepper.bIsCCW = s32Ticks > 0;

    ESP_ERROR_CHECK(esp_timer_start_once(this->m_stepper.sSignalTimerHandle, this->m_stepper.s32Period));

    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 30000 );

    /* Wait to be notified of an interrupt. */
    uint32_t ulNotifiedValue = 0;
    const BaseType_t xResult = xTaskNotifyWait(pdFALSE,    /* Don't clear bits on entry. */
                        ULONG_MAX,        /* Clear all bits on exit. */
                        &ulNotifiedValue, /* Stores the notified value. */
                        xMaxBlockTime );

    if( xResult != pdPASS )
    {
        esp_timer_stop(this->m_stepper.sSignalTimerHandle);
        ESP_LOGE(TAG, "Error, cannot reach it's destination with-in time ...");
        return false;
    }

    return true;
}

IRAM_ATTR void GateControl::tmr_signal_callback(void* arg)
{
    Stepper* step = (Stepper*)arg;

    static BaseType_t xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;

    const int32_t s32 = MISCMACRO_MIN(abs(step->s32Count) , abs(step->s32Target - step->s32Count));
    /* I just did some tests until I was satisfied */
    /* #1 (100, 1000), (1400, 300)
        a = -0.53846153846153846153846153846154
        b = 1053.84 */
    /* #2 (100, 700), (1400, 200)
        a = -0.3846
        b = 738.44 */
    const int32_t a = -461;
    const int32_t b = 946840;
    step->s32Period = (a * s32 + b)/1000;

    // I hoped it would reduce jitter.
    step->s32Period = (step->s32Period / 50) * 50;

    if (step->s32Period < 600)
        step->s32Period = 600;
    if (step->s32Period > 1600)
        step->s32Period = 1600;

    // Wait until the period go to low before considering it finished
    if (step->s32Target == step->s32Count)
    {
        xTaskNotifyFromISR( step->sTskControlHandle,
            STEPEND_BIT,
            eSetBits,
            &xHigherPriorityTaskWoken );
    }
    else {
        // Count every two
        if (step->bIsCCW)
            HW::getI()->StepStepperCCW();
        else
            HW::getI()->StepStepperCW();

        step->s32Count++;

        ESP_ERROR_CHECK(esp_timer_start_once(step->sSignalTimerHandle, step->s32Period));
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GateControl::AnimRampLight(bool bIsActive)
{
    const float fltPWMOn = (float)Settings::getI().GetValueInt32(Settings::Entry::RampOnPercent) / 100.0f;
    const float fltInc = 0.01f;

    if (bIsActive) {
        for(float flt = 0.0f; flt <= 1.0f; flt += fltInc) {
            // Log corrected
            HW::getI()->SetRampLight(MISCFA_LinearizeLEDOutput(flt)*fltPWMOn);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    else {
        for(float flt = 1.0f; flt >= 0.0f; flt -= fltInc) {
            HW::getI()->SetRampLight(MISCFA_LinearizeLEDOutput(flt)*fltPWMOn);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}
