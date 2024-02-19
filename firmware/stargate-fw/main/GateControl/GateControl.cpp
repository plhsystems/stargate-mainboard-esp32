#include <exception>
#include <stdexcept>
#include "GateControl.hpp"
#include "../Wormhole/Wormhole.hpp"
#include "../FWConfig.hpp"
#include "../HW/HW.hpp"
#include "misc-formula.h"
#include "SGURing.hpp"
#include "SGUComm.hpp"
#include "../Ring/RingComm.hpp"

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

void GateControl::QueueAutoHome()
{
    const SCmd sCmd = { .eCmd = ECmd::AutoHome };
    PriQueueAction(sCmd);
}

void GateControl::QueueAutoCalibrate()
{
    const SCmd sCmd = { .eCmd = ECmd::AutoCalibrate };
    PriQueueAction(sCmd);
}

void GateControl::QueueDialAddress(GateAddress& ga)
{
    const SCmd sCmd =
    {
        .eCmd = ECmd::DialAddress,
        .sDialAddress = { .sGateAddress = ga, .eWormholeType = Wormhole::EType::NormalSGU }
    };
    PriQueueAction(sCmd);
}

void GateControl::QueueManualWormhole(Wormhole::EType type)
{
    const SCmd sCmd =
    {
        .eCmd = ECmd::ManualWormhole,
        .sManualWormhole = { .eWormholeType = type }
    };
    PriQueueAction(sCmd);
}

void GateControl::AbortAction()
{
    m_bIsCancelAction = true;
}

void GateControl::PriQueueAction(SCmd cmd)
{
    m_sCmd = cmd;
}

void GateControl::TaskRunning(void* pArg)
{
    GateControl* gc = (GateControl*)pArg;

    ESP_LOGI(TAG, "Gatecontrol task started and ready.");

    // Dialing
    while(true)
    {
        const ECmd eCmd = gc->m_sCmd.eCmd;
        if (eCmd == ECmd::Idle)
        {
            // TODO: Will be replaced by a manual event.
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Reset the receiving ..
        gc->m_bIsCancelAction = false;
        gc->m_sCmd.eCmd = ECmd::Idle;

        try
        {
            switch(eCmd)
            {
                case ECmd::AutoCalibrate:
                {
                    ESP_LOGI(TAG, "Autocalibrate in progress.");
                    gc->AutoCalibrate();
                    // Will move it at it's home position, it should go very fast.
                    ESP_LOGI(TAG, "Autocalibrate succeeded.");
                    gc->AutoHome();
                    ESP_LOGI(TAG, "Auto-home succeeded.");
                    break;
                }
                case ECmd::AutoHome:
                {
                    ESP_LOGI(TAG, "Auto-home started.");
                    gc->AutoHome();
                    ESP_LOGI(TAG, "Auto-home succeeded.");
                    break;
                }
                case ECmd::KeyPress:
                {
                    // TODO: Keypress one by one
                    ESP_LOGI(TAG, "TODO: KeyPress");
                    break;
                }
                case ECmd::DialAddress:
                {
                    ESP_LOGI(TAG, "Dialing ....");
                    gc->DialAddress(gc->m_sCmd.sDialAddress.sGateAddress);
                    ESP_LOGI(TAG, "Dialing address succeeded.");
                    break;
                }
                case ECmd::ManualWormhole:
                {
                    ESP_LOGI(TAG, "ManualWormhole, name: %s", Wormhole::GetTypeText(gc->m_sCmd.sManualWormhole.eWormholeType));
                    Wormhole wm { HW::getI(), gc->m_sCmd.sManualWormhole.eWormholeType };
                    wm.Begin();
                    wm.OpeningAnimation();
                    while(!gc->m_bIsCancelAction) {
                        wm.RunTicks();
                    }
                    wm.ClosingAnimation();
                    wm.End();
                    break;
                }
                default:
                case ECmd::Idle:
                    break;
            }
        }
        catch(const std::exception& e)
        {
            ESP_LOGE(TAG, "Exception raised: %s", e.what());
        }

        // Reset at the end, it's not really a queue
        gc->m_bIsCancelAction = false;
        gc->m_sCmd.eCmd = ECmd::Idle;
    }
}

void GateControl::AutoCalibrate()
{
    const uint32_t u32Timeout = Settings::getI().GetValueInt32(Settings::Entry::RingCalibTimeout);

    try
    {
        // We need two transitions from LOW to HIGH.
        // we give it 40s maximum to find the home.
        HW::getI()->PowerUpStepper();

        ESP_LOGI(TAG, "Finding home in progress");
        SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, nullptr);
        ESP_LOGI(TAG, "Home has been found once");
        int32_t s32NewStepsPerRotation = 0;
        SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, &s32NewStepsPerRotation);

        ESP_LOGI(TAG, "Home has been found a second time, step: %" PRId32, s32NewStepsPerRotation);

        // Find the gap.
        // Continue to move until it get out of the home range.
        int32_t s32Gap = 0;

        SpinUntil(ESpinDirection::CCW, ETransition::Failing, u32Timeout, &s32Gap);
        SpinUntil(ESpinDirection::CW, ETransition::Rising, u32Timeout, &s32Gap);

        ESP_LOGI(TAG, "Ticks per rotation: %" PRId32 ", time per rotation, gap: % " PRId32, s32NewStepsPerRotation, s32Gap);

        // Save the calibration result.
        Settings::getI().SetValueInt32(Settings::Entry::StepsPerRotation, s32NewStepsPerRotation);
        Settings::getI().SetValueInt32(Settings::Entry::RingHomeGapRange, s32Gap);
        Settings::getI().Commit();

        // Go into the other direction until it get out of the sensor
        HW::getI()->PowerDownStepper();
    }
    catch(const std::exception& e)
    {
        // Go into the other direction until it get out of the sensor
        HW::getI()->PowerDownStepper();
        throw;
    }
}

void GateControl::AutoHome()
{
    try
    {
        HW::getI()->PowerUpStepper();

        const int32_t s32NewStepsPerRotation = Settings::getI().GetValueInt32(Settings::Entry::StepsPerRotation);
        const int32_t s32Gap = Settings::getI().GetValueInt32(Settings::Entry::RingHomeGapRange);
        if (s32NewStepsPerRotation == 0 || s32Gap == 0)
        {
            throw std::runtime_error("Auto-calibration needs to be done.");
        }

        const uint32_t u32Timeout = Settings::getI().GetValueInt32(Settings::Entry::RingCalibTimeout);

        // If the ring is already near the home sensor, we just need to move a little bit.
        if (HW::getI()->GetIsHomeSensorActive()) {
            ESP_LOGI(TAG, "Homing using the fast algorithm");

            SpinUntil(ESpinDirection::CW, ETransition::Failing, u32Timeout, nullptr);
            SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, nullptr);
        }
        else {
            ESP_LOGI(TAG, "Homing using the slow algorithm");
            SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, nullptr);
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

        // Go into the other direction until it get out of the sensor
        HW::getI()->PowerDownStepper();
    }
    catch(const std::exception& e)
    {
        // Go into the other direction until it get out of the sensor
        HW::getI()->PowerDownStepper();
        throw;
    }
}

void GateControl::DialAddress(GateAddress& ga)
{
    const int32_t s32NewStepsPerRotation = Settings::getI().GetValueInt32(Settings::Entry::StepsPerRotation);

    auto endOfProcess = [&](bool bIsError) -> void
    {
        // Go back to home position
        ESP_LOGI(TAG, "Move near the home position");
        const int32_t s32MoveTicks = MISCFA_CircleDiffd32(m_s32CurrentPositionTicks, 0, s32NewStepsPerRotation);
        MoveStepperTo(s32MoveTicks);
        ESP_LOGI(TAG, "Confirm the home position");
        AutoHome();

        vTaskDelay(pdMS_TO_TICKS(500));
        if (bIsError) {
            RingComm::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_ErrorToOff);
        } else {
            RingComm::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_FadeOut);
        }
        AnimRampLight(false);
        HW::getI()->PowerDownStepper();
    };

    try
    {
        Wormhole wm { HW::getI(), m_sCmd.sDialAddress.eWormholeType };
        HW::getI()->PowerUpStepper();

        if (!m_bIsHomingDone) {
            throw std::runtime_error("Homing need to be done");
        }

        AnimRampLight(true);
        vTaskDelay(pdMS_TO_TICKS(750));
        RingComm::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_FadeIn);
        vTaskDelay(pdMS_TO_TICKS(750));

        // const EChevron chevrons[] = { EChevron::Chevron1, EChevron::Chevron2, EChevron::Chevron3, EChevron::Chevron4, EChevron::Chevron5, EChevron::Chevron6, EChevron::Chevron7_Master, EChevron::Chevron8, EChevron::Chevron9 };
        for(int32_t i = 0; i < ga.GetSymbolCount(); i++)
        {
            if (m_bIsCancelAction) {
                throw std::runtime_error("Unable to complete dialing, cancelled by the user");
            }

            const uint8_t u8Symbol = ga.GetSymbol(i);

            // Dial sequence ...
            const int32_t s32LedIndex = SGURingNS::SymbolToLedIndex(u8Symbol);
            const double dAngle = (SGURingNS::LEDIndexToDeg(s32LedIndex));
            const int32_t s32SymbolToTicks = -1*(dAngle/360)*s32NewStepsPerRotation;

            const int32_t s32MoveTicks = MISCFA_CircleDiffd32(m_s32CurrentPositionTicks, s32SymbolToTicks, s32NewStepsPerRotation);

            ESP_LOGI(TAG, "led index: %" PRId32 ", angle: %.2f, symbol2Ticks: %" PRId32, s32LedIndex, dAngle, s32SymbolToTicks);
            MoveStepperTo(s32MoveTicks);

            vTaskDelay(pdMS_TO_TICKS(300));
            RingComm::getI().SendLightUpSymbol(u8Symbol);
            vTaskDelay(pdMS_TO_TICKS(750));

            m_s32CurrentPositionTicks = s32SymbolToTicks;
            vTaskDelay(pdMS_TO_TICKS(2000));
        }

        // Play the wormhole idling animation
        wm.Begin();
        wm.OpeningAnimation();
        while(!m_bIsCancelAction) {
            wm.RunTicks();
        }
        // Turn-off all symbols before killing the wormhole
        RingComm::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_NoSymbols);
        wm.ClosingAnimation();
        wm.End();

        endOfProcess(false);
    }
    catch(const std::exception& e)
    {
        endOfProcess(true);
        throw;
    }
}

void GateControl::SpinUntil(ESpinDirection eSpinDirection, ETransition eTransition, uint32_t u32TimeoutMS, int32_t* ps32refTickCount)
{
    TickType_t ttStart = xTaskGetTickCount();
    bool bOldSensorState = HW::getI()->GetIsHomeSensorActive();

    while ((xTaskGetTickCount() - ttStart) < pdMS_TO_TICKS(Settings::getI().GetValueInt32(Settings::Entry::RingCalibTimeout)))
    {
        if (m_bIsCancelAction) {
            throw std::runtime_error("Unable to complete the spin operation, cancelled by the user");
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
            ESP_LOGI(TAG, "Rising transition detected");
            return;
        }
        else if (ETransition::Failing == eTransition && bIsFalling) {
            ESP_LOGI(TAG, "Failing transition detected");
            return;
        }

        bOldSensorState = bNewHomeSensorState;
        vTaskDelay(1);
    }

    throw std::runtime_error("Unable to complete the spin operation");
}

void GateControl::MoveStepperTo(int32_t s32Ticks)
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
        throw std::runtime_error("Error, cannot reach it's destination with-in time ...");
    }
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
    const int32_t a = -400;
    const int32_t b = 1400000;
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
    const float fltInc = 0.005f;

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
