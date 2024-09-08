#include <exception>
#include <stdexcept>
#include "GateControl.hpp"
#include "../Wormhole/Wormhole.hpp"
#include "../FWConfig.hpp"
#include "misc-formula.h"
#include "SGURing.hpp"
#include "SGUComm.hpp"
#include "../Ring/RingComm.hpp"

#define TAG "GateControl"

GateControl::GateControl() :
    m_bIsCancelAction(false)
{
    m_xSemaphoreHandle = xSemaphoreCreateMutexStatic(&m_xSemaphoreCreateMutex);
}

void GateControl::Init(SGHW_HAL* pSGHWHal)
{
    m_pSGHWHAL = pSGHWHal;
}

void GateControl::StartTask()
{
	if (xTaskCreatePinnedToCore(TaskRunning, "GateControl", FWCONFIG_GATECONTROL_STACKSIZE, (void*)this, FWCONFIG_GATECONTROL_PRIORITY_DEFAULT, &m_sGateControlHandle, FWCONFIG_GATECONTROL_COREID) != pdPASS )
	{
		ESP_ERROR_CHECK(ESP_FAIL);
	}
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
    m_sNextCmd = cmd;
}

void GateControl::TaskRunning(void* pArg)
{
    GateControl* gc = (GateControl*)pArg;

    ESP_LOGI(TAG, "Gatecontrol task started and ready.");

    // Dialing
    while(true)
    {
        xSemaphoreTake(gc->m_xSemaphoreHandle, portMAX_DELAY);
        gc->m_sCurrCmd = gc->m_sNextCmd;
        gc->m_bIsCancelAction = false;
        gc->m_sNextCmd.eCmd = ECmd::Idle;   // Reset the command "queue"
        xSemaphoreGive(gc->m_xSemaphoreHandle);

        if (gc->m_sCurrCmd.eCmd == ECmd::Idle)
        {
            // TODO: Will be replaced by a manual event.
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Reset the receiving ..
        xSemaphoreTake(gc->m_xSemaphoreHandle, portMAX_DELAY);
        gc->m_szErrors[0] = '\0';
        gc->m_bIsInError = false;
        xSemaphoreGive(gc->m_xSemaphoreHandle);

        try
        {
            switch(gc->m_sCurrCmd.eCmd)
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
                    gc->DialAddress(gc->m_sCurrCmd.sDialAddress);
                    ESP_LOGI(TAG, "Dialing address succeeded.");
                    break;
                }
                case ECmd::ManualWormhole:
                {
                    ESP_LOGI(TAG, "ManualWormhole, name: %s", Wormhole::GetTypeText(gc->m_sCurrCmd.sManualWormhole.eWormholeType));
                    Wormhole wm { gc->m_pSGHWHAL, gc->m_sCurrCmd.sManualWormhole.eWormholeType };
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

            // To be displayed into the web page.
            xSemaphoreTake(gc->m_xSemaphoreHandle, portMAX_DELAY);
            strncpy(gc->m_szErrors, e.what(), ERROR_LEN);
            gc->m_bIsInError = true;
            xSemaphoreGive(gc->m_xSemaphoreHandle);
        }

        // Reset at the end, it's not really a queue
        gc->m_bIsCancelAction = false;
        gc->m_sCurrCmd.eCmd = ECmd::Idle;
    }
}

void GateControl::AutoCalibrate()
{
    const uint32_t u32Timeout = Settings::getI().GetValueInt32(Settings::Entry::RingCalibTimeout);

    try
    {
        // We need two transitions from LOW to HIGH.
        // we give it 40s maximum to find the home.
        m_pSGHWHAL->PowerUpStepper();
        ReleaseClamp();

        ESP_LOGI(TAG, "Finding home in progress");
        m_pSGHWHAL->SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, nullptr);
        ESP_LOGI(TAG, "Home has been found once");
        int32_t s32NewStepsPerRotation = 0;
        m_pSGHWHAL->SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, &s32NewStepsPerRotation);

        ESP_LOGI(TAG, "Home has been found a second time, step: %" PRId32, s32NewStepsPerRotation);

        // Find the gap.
        // Continue to move until it get out of the home range.
        int32_t s32Gap = 0;

        m_pSGHWHAL->SpinUntil(ESpinDirection::CCW, ETransition::Failing, u32Timeout, &s32Gap);
        m_pSGHWHAL->SpinUntil(ESpinDirection::CW, ETransition::Rising, u32Timeout, &s32Gap);

        ESP_LOGI(TAG, "Ticks per rotation: %" PRId32 ", time per rotation, gap: % " PRId32, s32NewStepsPerRotation, s32Gap);

        // Save the calibration result.
        Settings::getI().SetValueInt32(Settings::Entry::StepsPerRotation, s32NewStepsPerRotation);
        Settings::getI().SetValueInt32(Settings::Entry::RingHomeGapRange, s32Gap);
        Settings::getI().Commit();

        // Go into the other direction until it get out of the sensor
        LockClamp();
        m_pSGHWHAL->PowerDownStepper();
        m_pSGHWHAL->PowerDownServo();
    }
    catch(const std::exception& e)
    {
        // Go into the other direction until it get out of the sensor
        m_pSGHWHAL->PowerDownStepper();
        LockClamp();
        throw;
    }
}

void GateControl::AutoHome()
{
    try
    {
        m_pSGHWHAL->PowerUpStepper();
        ReleaseClamp();

        const int32_t s32NewStepsPerRotation = Settings::getI().GetValueInt32(Settings::Entry::StepsPerRotation);
        const int32_t s32Gap = Settings::getI().GetValueInt32(Settings::Entry::RingHomeGapRange);
        if (s32NewStepsPerRotation == 0 || s32Gap == 0)
        {
            throw std::runtime_error("Auto-calibration needs to be done");
        }

        const uint32_t u32Timeout = Settings::getI().GetValueInt32(Settings::Entry::RingCalibTimeout);

        // If the ring is already near the home sensor, we just need to move a little bit.
        if (m_pSGHWHAL->GetIsHomeSensorActive()) {
            ESP_LOGI(TAG, "Homing using the fast algorithm");

            m_pSGHWHAL->SpinUntil(ESpinDirection::CW, ETransition::Failing, u32Timeout, nullptr);
            m_pSGHWHAL->SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, nullptr);
        }
        else {
            ESP_LOGI(TAG, "Homing using the slow algorithm");
            m_pSGHWHAL->SpinUntil(ESpinDirection::CCW, ETransition::Rising, u32Timeout, nullptr);
        }

        // Move by half the deadband offset.
        // this is the real 0 position
        const int32_t s32HalfDeadband = s32Gap / 2;
        ESP_LOGI(TAG, "Moving a little bit to take care of the deadband, offset: %" PRId32, s32HalfDeadband);
        for(int i = 0; i < s32HalfDeadband; i++)
        {
            m_pSGHWHAL->StepStepperCCW();
            vTaskDelay(1);
        }

        m_s32CurrentPositionTicks = 0;
        m_bIsHomingDone = true;

        LockClamp();
        // Go into the other direction until it get out of the sensor
        m_pSGHWHAL->PowerDownStepper();
    }
    catch(const std::exception& e)
    {
        LockClamp();
        // Go into the other direction until it get out of the sensor
        m_pSGHWHAL->PowerDownStepper();
        throw;
    }
}

void GateControl::DialAddress(const SDialArg& sDialArg)
{
    const int32_t s32NewStepsPerRotation = Settings::getI().GetValueInt32(Settings::Entry::StepsPerRotation);

    auto endOfProcess = [&](bool bIsError) -> void
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        if (bIsError) {
            RingComm::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_ErrorToOff);
        } else {
            RingComm::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_FadeOut);
        }
        AnimRampLight(false);

        if (!bIsError) {
            // If no error happened, just wait a little bit for the effect.
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
        // Go back to home position
        ESP_LOGI(TAG, "Move near the home position");
        const int32_t s32MoveTicks = MISCFA_CircleDiffd32(m_s32CurrentPositionTicks, 0, s32NewStepsPerRotation);
        m_pSGHWHAL->MoveStepperTo(s32MoveTicks, 30000);
        ESP_LOGI(TAG, "Confirm the home position");
        AutoHome();

        m_pSGHWHAL->PowerDownStepper();
        LockClamp();
    };

    try
    {
        Wormhole wm { m_pSGHWHAL, sDialArg.eWormholeType };
        m_pSGHWHAL->PowerUpStepper();
        ReleaseClamp();

        if (!m_bIsHomingDone) {
            throw std::runtime_error("Homing need to be done");
        }

        AnimRampLight(true);
        vTaskDelay(pdMS_TO_TICKS(750));
        RingComm::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_FadeIn);
        vTaskDelay(pdMS_TO_TICKS(750));

        // const EChevron chevrons[] = { EChevron::Chevron1, EChevron::Chevron2, EChevron::Chevron3, EChevron::Chevron4, EChevron::Chevron5, EChevron::Chevron6, EChevron::Chevron7_Master, EChevron::Chevron8, EChevron::Chevron9 };
        for(int32_t i = 0; i < sDialArg.sGateAddress.GetSymbolCount(); i++)
        {
            if (m_bIsCancelAction) {
                throw std::runtime_error("Cancelled by the user");
            }

            const uint8_t u8Symbol = sDialArg.sGateAddress.GetSymbol(i);

            // Dial sequence ...
            const int32_t s32LedIndex = SGURingNS::SymbolToLedIndex(u8Symbol);
            const double dAngle = (SGURingNS::LEDIndexToDeg(s32LedIndex));
            const int32_t s32SymbolToTicks = -1*(dAngle/360)*s32NewStepsPerRotation;

            const int32_t s32MoveTicks = MISCFA_CircleDiffd32(m_s32CurrentPositionTicks, s32SymbolToTicks, s32NewStepsPerRotation);

            ESP_LOGI(TAG, "led index: %" PRId32 ", angle: %.2f, symbol2Ticks: %" PRId32, s32LedIndex, dAngle, s32SymbolToTicks);
            m_pSGHWHAL->MoveStepperTo(s32MoveTicks, 30000);

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


void GateControl::AnimRampLight(bool bIsActive)
{
    const float fltPWMOn = (float)Settings::getI().GetValueInt32(Settings::Entry::RampOnPercent) / 100.0f;
    const float fltInc = 0.005f;

    if (bIsActive) {
        for(float flt = 0.0f; flt <= 1.0f; flt += fltInc) {
            // Log corrected
            m_pSGHWHAL->SetRampLight(MISCFA_LinearizeLEDOutput(flt)*fltPWMOn);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    else {
        for(float flt = 1.0f; flt >= 0.0f; flt -= fltInc) {
            m_pSGHWHAL->SetRampLight(MISCFA_LinearizeLEDOutput(flt)*fltPWMOn);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

void GateControl::GetState(UIState& uiState)
{
    xSemaphoreTake(m_xSemaphoreHandle, portMAX_DELAY);
    uiState.eCmd = m_sCurrCmd.eCmd;
    // Last error
    uiState.bHasLastError = m_bIsInError;
    strcpy(uiState.szLastError, m_szErrors);

    strcpy(uiState.szStatusText, GetCmdText(m_sCurrCmd.eCmd));

    uiState.bIsCancelRequested = m_bIsCancelAction;
    xSemaphoreGive(m_xSemaphoreHandle);
}

void GateControl::ReleaseClamp()
{
    // Release the clamp
    m_pSGHWHAL->PowerUpServo();
    m_pSGHWHAL->SetServo(Settings::getI().GetValueDouble(Settings::Entry::ClampReleasedPWM));
    vTaskDelay(pdMS_TO_TICKS(500));
}

void GateControl::LockClamp()
{
    m_pSGHWHAL->PowerUpServo();
    m_pSGHWHAL->SetServo(Settings::getI().GetValueDouble(Settings::Entry::ClampLockedPWM));
    vTaskDelay(pdMS_TO_TICKS(500));
    m_pSGHWHAL->PowerDownServo();
}

