#include "GateControl.hpp"
#include "Wormhole/Wormhole.hpp"
#include "FWConfig.hpp"
#include "HW/HW.hpp"

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
                gc->m_eCmd = ECmd::Idle;
                break;
            }
            default:
            case ECmd::Idle:
                break;
        }

        // TODO: Will be replaced by a manual event.
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

bool GateControl::AutoCalibrate()
{
    bool bSucceeded = false;
    {
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
    ttStart = xTaskGetTickCount();
    while ((xTaskGetTickCount() - ttStart) < pdMS_TO_TICKS(40*1000))
    {
        const bool bCurrentHome = HW::getI()->GetIsHomeSensorActive();
        if (bOldHome && !bCurrentHome)
        {
            ESP_LOGI(TAG, "Moved out of range");
            break;
        }
        HW::getI()->StepStepperCCW();
        s32Gap++;
        bOldHome = bCurrentHome;
        vTaskDelay(10);
    }

    if (s32Gap == 0) {
        ESP_LOGE(TAG, "Timeout condition during find gap phase #1");
        goto ERROR;
    }

    // Invert rotation until it reach the home range again.
    ttStart = xTaskGetTickCount();
    bool bIsGapFound = false;
    while ((xTaskGetTickCount() - ttStart) < pdMS_TO_TICKS(40*1000))
    {
        const bool bCurrentHome = HW::getI()->GetIsHomeSensorActive();
        if (!bOldHome && bCurrentHome)
        {
            ESP_LOGI(TAG, "With-in range");
            bIsGapFound = true;
            break;
        }
        HW::getI()->StepStepperCW();
        s32Gap--;
        bOldHome = bCurrentHome;
        vTaskDelay(10);
    }

    if (!bIsGapFound) {
        ESP_LOGE(TAG, "Timeout condition during find gap phase #2");
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