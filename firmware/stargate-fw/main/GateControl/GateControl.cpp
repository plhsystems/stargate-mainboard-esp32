#include "GateControl.hpp"
#include "../Wormhole/Wormhole.hpp"
#include "../FWConfig.hpp"
#include "../HW/HW.hpp"

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
	if (xTaskCreatePinnedToCore(TaskRunning, "GateControl", FWCONFIG_GATECONTROL_STACKSIZE, (void*)this, FWCONFIG_GATECONTROL_PRIORITY_DEFAULT, &m_sGateControlHandle, FWCONFIG_GATECONTROL_COREID) != pdPASS )
	{
		ESP_ERROR_CHECK(ESP_FAIL);
	}
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
    BaseGate& baseGate = GateFactory::Get(GateGalaxy::MilkyWay);

    Wormhole wormhole(Wormhole::EType::NormalSG1);

    // Dialing
    while(true)
    {
        gc->m_bIsCancelAction = false;

        switch(gc->m_eCmd)
        {
            case ECmd::AutoCalibrate:
            {
                ESP_LOGI(TAG, "Autocalibrate in progress.");
                if (!gc->AutoCalibrate()) {
                    ESP_LOGE(TAG, "Autocalibration failed.");
                }
                else {
                    ESP_LOGI(TAG, "Autocalibrate succeeded.");
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
            default:
            case ECmd::Idle:
                break;
        }

        // TODO: Will be replaced by a manual event.
        gc->m_eCmd = ECmd::Idle;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

bool GateControl::AutoCalibrate()
{
    bool bSucceeded = false;
    {
    const uint32_t u32Timeout = 40*1000;

    // We need two transitions from LOW to HIGH.
    // we give it 40s maximum to find the home.
    TickType_t ttStart = xTaskGetTickCount();

    bool bOldHome = HW::getI()->GetIsHomeSensorActive();
    int32_t s32TickCount = 0;
    bool bIsFound = false;

    HW::getI()->PowerUpStepper();
    vTaskDelay(pdMS_TO_TICKS(100));

    int32_t s32NewStepsPerRotation = 0;

    while ((xTaskGetTickCount() - ttStart) < pdMS_TO_TICKS(40*1000))
    {
        const bool bCurrentHome = HW::getI()->GetIsHomeSensorActive();

        // Transition from LOW to HIGH.
        if (!bOldHome && bCurrentHome)
        {
            if (bIsFound) {
                // Second stage done.
                s32NewStepsPerRotation = s32TickCount;
                break;
            }
            else {
                ttStart = xTaskGetTickCount(); // Reset the timer.
                s32TickCount = 0;
                bIsFound = true;
                ESP_LOGI(TAG, "Home has been found once.");
            }
        }

        bOldHome = bCurrentHome;
        s32TickCount++;
        HW::getI()->StepStepperCCW();
        vTaskDelay(1);
    }

    // Save this information
    if (s32NewStepsPerRotation == 0) {
        ESP_LOGE(TAG, "Timeout condition during auto-calibrate");
        goto ERROR;
    }

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

bool GateControl::SpinUntil(ESpinDirection eSpinDirection, ETransition eTransition, uint32_t u32TimeoutMS, int32_t* ps32refTickCount)
{
    TickType_t ttStart = xTaskGetTickCount();
    const bool bOldSensorState = HW::getI()->GetIsHomeSensorActive();

    while ((xTaskGetTickCount() - ttStart) < pdMS_TO_TICKS(40*1000))
    {
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

        const bool bIsRising = !bOldSensorState && HW::getI()->GetIsHomeSensorActive();
        const bool bIsFalling = bOldSensorState && !HW::getI()->GetIsHomeSensorActive();
        if (ETransition::Rising == eTransition && bIsRising)
        {
            ESP_LOGI(TAG, "Rising transistion detected");
            return true;
        }
        else if (ETransition::Failing == eTransition && bIsFalling) {
            ESP_LOGI(TAG, "Failing transistion detected");
            return true;
        }
        vTaskDelay(1);
    }

    ESP_LOGE(TAG, "Unable to complete the spin operation");
    return false;
}

/*
    //baseGate.UnlockGate(); //Only apply on universe
    //baseGate.GoHome();

    const Chevron chevrons[] = { Chevron::Chevron1, Chevron::Chevron2, Chevron::Chevron3, Chevron::Chevron4, Chevron::Chevron5, Chevron::Chevron6, Chevron::Chevron7_Master };
    const uint8_t symbols[] = { 5, 10, 20, 30, 3, 16, 1 };

    for(int32_t i = 0; i < (sizeof(symbols)/sizeof(symbols[0])); i++)
    {
        const uint8_t symbol = symbols[i];
        const Chevron currentChevron = chevrons[i];

        //baseGate.MoveToSymbol(symbol, currentChevron);
        // Chevrons
        //baseGate.LockChevron();
        vTaskDelay(pdMS_TO_TICKS(500));
        //baseGate.UnlockChevron();

        //baseGate.LightUpSymbol(20, true); //Only apply on universe
        vTaskDelay(pdMS_TO_TICKS(500));
        //baseGate.LightUpChevron(currentChevron, true);
    }

    // Run wormhole animations
    wormhole.OpenAnimation();
    wormhole.Run(&gc->m_bIsCancelAction);
    wormhole.CloseAnimation();

    // Shutdown lightning and all
    //baseGate.ShutdownGate();

    //baseGate.GoHome();

    //baseGate.LockGate(); //Only apply on universe
 */