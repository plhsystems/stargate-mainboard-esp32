#include "GateControl.hpp"
#include "../Wormhole/Wormhole.hpp"
#include "../FWConfig.hpp"
#include "misc-formula.h"
#include "SGURing.hpp"
#include "SGUComm.hpp"
#include "../Ring/RingBLEClient.hpp"
#include "../Audio/SoundFX.hpp"

#define TAG "GateControl"

GateControl::GateControl() :
    m_is_cancel_action(false)
{
    m_semaphore_handle = xSemaphoreCreateMutexStatic(&m_semaphore_create_mutex);
}

void GateControl::Init(SGHW_HAL* sghw_hal)
{
    m_sghw_hal = sghw_hal;
}

void GateControl::StartTask()
{
	if (xTaskCreatePinnedToCore(TaskRunning, "GateControl", FWCONFIG_GATECONTROL_STACKSIZE, (void*)this, FWCONFIG_GATECONTROL_PRIORITY_DEFAULT, &m_gate_control_handle, FWCONFIG_GATECONTROL_COREID) != pdPASS )
	{
		ESP_ERROR_CHECK(ESP_FAIL);
	}
}

void GateControl::QueueAutoHome()
{
    const SCmd command = { .cmd = ECmd::AutoHome };
    PriQueueAction(command);
}

void GateControl::QueueAutoCalibrate()
{
    const SCmd cmd = { .cmd = ECmd::AutoCalibrate };
    PriQueueAction(cmd);
}

void GateControl::QueueDialAddress(GateAddress& ga)
{
    const SCmd cmd =
    {
        .cmd = ECmd::DialAddress,
        .dial_address = { .gate_address = ga, .wormhole_type = Wormhole::EType::NormalSGU }
    };
    PriQueueAction(cmd);
}

void GateControl::QueueManualWormhole(Wormhole::EType type)
{
    const SCmd cmd =
    {
        .cmd = ECmd::ManualWormhole,
        .manual_wormhole = { .wormhole_type = type }
    };
    PriQueueAction(cmd);
}

void GateControl::AbortAction()
{
    m_is_cancel_action = true;
}

void GateControl::PriQueueAction(SCmd cmd)
{
    m_next_cmd = cmd;
}

void GateControl::TaskRunning(void* arg)
{
    GateControl* gc = (GateControl*)arg;

    ESP_LOGI(TAG, "Gatecontrol task started and ready.");

    // Dialing
    while(true)
    {
        xSemaphoreTake(gc->m_semaphore_handle, portMAX_DELAY);
        gc->m_curr_cmd = gc->m_next_cmd;
        gc->m_is_cancel_action = false;
        gc->m_next_cmd.cmd = ECmd::Idle;   // Reset the command "queue"
        xSemaphoreGive(gc->m_semaphore_handle);

        if (gc->m_curr_cmd.cmd == ECmd::Idle)
        {
            // TODO: Will be replaced by a manual event.
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Reset the receiving ..
        xSemaphoreTake(gc->m_semaphore_handle, portMAX_DELAY);
        gc->m_errors[0] = '\0';
        gc->m_is_in_error = false;
        gc->m_last_error_code = HWError::Ok;
        xSemaphoreGive(gc->m_semaphore_handle);

        VoidResult result = VoidResult::success();

        do {
            switch(gc->m_curr_cmd.cmd)
            {
                case ECmd::AutoCalibrate:
                {
                    gc->m_state_machine.processEvent(GateEvent::CmdCalibrate);
                    ESP_LOGI(TAG, "Autocalibrate in progress.");
                    result = gc->AutoCalibrate();
                    if (result.failed()) {
                        gc->m_state_machine.processEvent(GateEvent::OperationFailed);
                        break;
                    }
                    // Will move it at it's home position, it should go very fast.
                    ESP_LOGI(TAG, "Autocalibrate succeeded.");
                    gc->m_state_machine.processEvent(GateEvent::OperationComplete);

                    gc->m_state_machine.processEvent(GateEvent::CmdHome);
                    result = gc->AutoHome();
                    if (result.failed()) {
                        gc->m_state_machine.processEvent(GateEvent::OperationFailed);
                        break;
                    }
                    ESP_LOGI(TAG, "Auto-home succeeded.");
                    gc->m_state_machine.processEvent(GateEvent::OperationComplete);
                    break;
                }
                case ECmd::AutoHome:
                {
                    gc->m_state_machine.processEvent(GateEvent::CmdHome);
                    ESP_LOGI(TAG, "Auto-home started.");
                    result = gc->AutoHome();
                    if (result.failed()) {
                        gc->m_state_machine.processEvent(GateEvent::OperationFailed);
                        break;
                    }
                    ESP_LOGI(TAG, "Auto-home succeeded.");
                    gc->m_state_machine.processEvent(GateEvent::OperationComplete);
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
                    gc->m_state_machine.processEvent(GateEvent::CmdDial);
                    ESP_LOGI(TAG, "Dialing ....");
                    result = gc->DialAddress(gc->m_curr_cmd.dial_address);
                    if (result.failed()) {
                        gc->m_state_machine.processEvent(GateEvent::OperationFailed);
                        break;
                    }
                    ESP_LOGI(TAG, "Dialing address succeeded.");
                    gc->m_state_machine.processEvent(GateEvent::OperationComplete);
                    break;
                }
                case ECmd::ManualWormhole:
                {
                    gc->m_state_machine.processEvent(GateEvent::CmdManualWormhole);
                    ESP_LOGI(TAG, "ManualWormhole, name: %s", Wormhole::GetTypeText(gc->m_curr_cmd.manual_wormhole.wormhole_type));
                    Wormhole wm { gc->m_sghw_hal, gc->m_curr_cmd.manual_wormhole.wormhole_type };
                    wm.Begin();
                    wm.OpeningAnimation();
                    while(!gc->m_is_cancel_action) {
                        // Unlimited time, it violate laws of physics! (AKA the needs of the plot)
                        if (!wm.RunTicks()) {
                            result = HWError::HardwareFailure;
                            break;
                        }
                    }
                    wm.ClosingAnimation();
                    wm.End();
                    if (gc->m_is_cancel_action) {
                        gc->m_state_machine.processEvent(GateEvent::CmdAbort);
                    }
                    gc->m_state_machine.processEvent(GateEvent::OperationComplete);
                    break;
                }
                default:
                case ECmd::Idle:
                    break;
            }
        } while(false);

        if (result.failed())
        {
            ESP_LOGE(TAG, "Error occurred: %s (code: %d)", result.errorString(), (int)result.error());

            // To be displayed into the web page.
            xSemaphoreTake(gc->m_semaphore_handle, portMAX_DELAY);
            strncpy(gc->m_errors, result.errorString(), ERROR_LEN);
            gc->m_last_error_code = result.error();
            gc->m_is_in_error = true;
            xSemaphoreGive(gc->m_semaphore_handle);
        }

        // Reset at the end, it's not really a queue
        gc->m_is_cancel_action = false;
        gc->m_curr_cmd.cmd = ECmd::Idle;
    }
}

VoidResult GateControl::AutoCalibrate()
{
    const uint32_t timeout = Settings::getI().GetValueInt32(Settings::Entry::RingCalibTimeout);
    VoidResult result = HWError::Timeout;

    do {
        // We need two transitions from LOW to HIGH.
        // we give it 40s maximum to find the home.
        m_sghw_hal->PowerUpStepper();
        ReleaseClamp();

        ESP_LOGI(TAG, "Finding home in progress");
        if (!m_sghw_hal->SpinUntil(ESpinDirection::CCW, ETransition::Rising, timeout, nullptr)) {
            ESP_LOGE(TAG, "Calibration failed: timeout finding first home position");
            result = HWError::Timeout;
            break;
        }

        ESP_LOGI(TAG, "Home has been found once");
        int32_t new_steps_per_rotation = 0;
        if (!m_sghw_hal->SpinUntil(ESpinDirection::CCW, ETransition::Rising, timeout, &new_steps_per_rotation)) {
            ESP_LOGE(TAG, "Calibration failed: timeout finding second home position");
            result = HWError::Timeout;
            break;
        }

        ESP_LOGI(TAG, "Home has been found a second time, step: %" PRId32, new_steps_per_rotation);

        // Find the gap.
        // Continue to move until it get out of the home range.
        int32_t gap = 0;

        if (!m_sghw_hal->SpinUntil(ESpinDirection::CCW, ETransition::Failing, timeout, &gap)) {
            ESP_LOGE(TAG, "Calibration failed: timeout measuring gap (failing edge)");
            result = HWError::Timeout;
            break;
        }
        if (!m_sghw_hal->SpinUntil(ESpinDirection::CW, ETransition::Rising, timeout, &gap)) {
            ESP_LOGE(TAG, "Calibration failed: timeout measuring gap (rising edge)");
            result = HWError::Timeout;
            break;
        }

        ESP_LOGI(TAG, "Ticks per rotation: %" PRId32 ", time per rotation, gap: % " PRId32, new_steps_per_rotation, gap);

        // Save the calibration result.
        Settings::getI().SetValueInt32(Settings::Entry::StepsPerRotation, new_steps_per_rotation);
        Settings::getI().SetValueInt32(Settings::Entry::RingHomeGapRange, gap);
        Settings::getI().Commit();

        // Go into the other direction until it get out of the sensor
        LockClamp();
        m_sghw_hal->PowerDownStepper();
        m_sghw_hal->PowerDownServo();

        result = VoidResult::success();
    } while(false);

    if (result.failed()) {
        // Cleanup on error
        m_sghw_hal->PowerDownStepper();
        LockClamp();
    }

    return result;
}

VoidResult GateControl::AutoHome()
{
    VoidResult result = HWError::Timeout;

    do {
        m_sghw_hal->PowerUpStepper();
        ReleaseClamp();

        const int32_t new_steps_per_rotation = Settings::getI().GetValueInt32(Settings::Entry::StepsPerRotation);
        const int32_t gap = Settings::getI().GetValueInt32(Settings::Entry::RingHomeGapRange);
        if (new_steps_per_rotation == 0 || gap == 0)
        {
            ESP_LOGE(TAG, "Homing failed: calibration not done");
            result = HWError::NotCalibrated;
            break;
        }

        const uint32_t timeout = Settings::getI().GetValueInt32(Settings::Entry::RingCalibTimeout);

        // If the ring is already near the home sensor, we just need to move a little bit.
        if (m_sghw_hal->GetIsHomeSensorActive()) {
            ESP_LOGI(TAG, "Homing using the fast algorithm");

            if (!m_sghw_hal->SpinUntil(ESpinDirection::CW, ETransition::Failing, timeout, nullptr)) {
                ESP_LOGE(TAG, "Homing failed: timeout exiting home zone");
                result = HWError::Timeout;
                break;
            }
            if (!m_sghw_hal->SpinUntil(ESpinDirection::CCW, ETransition::Rising, timeout, nullptr)) {
                ESP_LOGE(TAG, "Homing failed: timeout re-entering home zone");
                result = HWError::Timeout;
                break;
            }
        }
        else {
            ESP_LOGI(TAG, "Homing using the slow algorithm");
            if (!m_sghw_hal->SpinUntil(ESpinDirection::CCW, ETransition::Rising, timeout, nullptr)) {
                ESP_LOGE(TAG, "Homing failed: timeout searching for home zone");
                result = HWError::Timeout;
                break;
            }
        }

        // Move by half the deadband offset.
        // this is the real 0 position
        const int32_t half_deadband = gap / 2;
        ESP_LOGI(TAG, "Moving a little bit to take care of the deadband, offset: %" PRId32, half_deadband);
        for(int i = 0; i < half_deadband; i++)
        {
            m_sghw_hal->StepStepperCCW();
            vTaskDelay(1);
        }

        m_current_position_ticks = 0;
        m_is_homing_done = true;

        LockClamp();
        // Go into the other direction until it get out of the sensor
        m_sghw_hal->PowerDownStepper();

        result = VoidResult::success();
    } while(false);

    if (result.failed()) {
        LockClamp();
        // Go into the other direction until it get out of the sensor
        m_sghw_hal->PowerDownStepper();
    }

    return result;
}

VoidResult GateControl::DialAddress(const SDialArg& dial_arg)
{
    const int32_t new_steps_per_rotation = Settings::getI().GetValueInt32(Settings::Entry::StepsPerRotation);
    VoidResult result = HWError::HardwareFailure;
    bool process_started = false;

    auto endOfProcess = [&](bool is_error) -> void
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        if (is_error) {
            RingBLEClient::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_ErrorToOff);
            SoundFX::getI().PlaySound(SoundFX::FileID::SGU_7_lockfail, false);
        } else {
            RingBLEClient::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_FadeOut);
        }
        AnimRampLight(false);

        if (!is_error) {
            // If no error happened, just wait a little bit for the effect.
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
        // Go back to home position
        ESP_LOGI(TAG, "Move near the home position");
        const int32_t move_ticks = MISCFA_CircleDiffd32(m_current_position_ticks, 0, new_steps_per_rotation);
        if (!m_sghw_hal->MoveStepperTo(move_ticks, 30000)) {
            return;
        }

        ESP_LOGI(TAG, "Confirm the home position");
        AutoHome();

        m_sghw_hal->PowerDownStepper();
        LockClamp();
    };

    do
    {
        Wormhole wm { m_sghw_hal, dial_arg.wormhole_type };
        SoundFX::getI().StopSound();
        m_sghw_hal->PowerUpStepper();
        ReleaseClamp();
        process_started = true;

        if (!m_is_homing_done) {
            ESP_LOGE(TAG, "Dial failed: homing not done");
            result = HWError::NotHomed;
            break;
        }

        AnimRampLight(true);
        vTaskDelay(pdMS_TO_TICKS(750));
        SoundFX::getI().PlaySound(SoundFX::FileID::SGU_1_beginroll, false);
        RingBLEClient::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_FadeIn);
        vTaskDelay(pdMS_TO_TICKS(750));

        bool dial_loop_ok = true;
        // const EChevron chevrons[] = { EChevron::Chevron1, EChevron::Chevron2, EChevron::Chevron3, EChevron::Chevron4, EChevron::Chevron5, EChevron::Chevron6, EChevron::Chevron7_Master, EChevron::Chevron8, EChevron::Chevron9 };
        for(int32_t i = 0; i < dial_arg.gate_address.GetSymbolCount(); i++)
        {
            if (m_is_cancel_action) {
                ESP_LOGW(TAG, "Dial cancelled by user at symbol %d", (int)i);
                result = HWError::Cancelled;
                dial_loop_ok = false;
                break;
            }

            const uint8_t symbol = dial_arg.gate_address.GetSymbol(i);

            // Dial sequence ...
            const int32_t led_index = SGURingNS::SymbolToLedIndex(symbol);
            const double angle = (SGURingNS::LEDIndexToDeg(led_index));
            const int32_t symbol_to_ticks = -1*(angle/360)*new_steps_per_rotation;

            const int32_t move_ticks = MISCFA_CircleDiffd32(m_current_position_ticks, symbol_to_ticks, new_steps_per_rotation);

            ESP_LOGI(TAG, "led index: %" PRId32 ", angle: %.2f, symbol2Ticks: %" PRId32, led_index, angle, symbol_to_ticks);
            SoundFX::getI().PlaySound(SoundFX::FileID::SGU_6_lggroll, true);
            vTaskDelay(pdMS_TO_TICKS(250));
            if (!m_sghw_hal->MoveStepperTo(move_ticks, 30000)) {
                ESP_LOGE(TAG, "Dial failed: motor timeout at symbol %d", (int)i);
                result = HWError::Timeout;
                dial_loop_ok = false;
                break;
            }
            SoundFX::getI().StopSound();
            SoundFX::getI().PlaySound(SoundFX::FileID::SGU_3_chevlck2, false);
            vTaskDelay(pdMS_TO_TICKS(1000));
            SoundFX::getI().PlaySound(SoundFX::FileID::SGU_2_chevlck, false);
            RingBLEClient::getI().SendLightUpSymbol(symbol);

            m_current_position_ticks = symbol_to_ticks;
            vTaskDelay(pdMS_TO_TICKS(2000));
        }

        if (!dial_loop_ok || m_is_cancel_action) {
            if (m_is_cancel_action && result.ok()) {
                result = HWError::Cancelled;
            }
            break;
        }

        // Play the wormhole idling animation
        SoundFX::getI().PlaySound(SoundFX::FileID::SGU_5_gateopen, false);
        wm.Begin();
        wm.OpeningAnimation();

        const uint32_t start_ticks = xTaskGetTickCount();
        while(!m_is_cancel_action)
        {
            // 5 minutes
            if ( (xTaskGetTickCount() - start_ticks) > pdMS_TO_TICKS(5*60*1000) ) {
                break;
            }
            if (!wm.RunTicks()) {
                break;
            }
        }
        // Turn-off all symbols before killing the wormhole
        SoundFX::getI().PlaySound(SoundFX::FileID::SGU_4_gateclos, false);
        RingBLEClient::getI().SendGateAnimation(SGUCommNS::EChevronAnimation::Chevron_NoSymbols);
        vTaskDelay(pdMS_TO_TICKS(1000));
        wm.ClosingAnimation();
        wm.End();

        result = VoidResult::success();
    } while(false);

    if (process_started) {
        endOfProcess(result.failed());
    }

    return result;
}


void GateControl::AnimRampLight(bool is_active)
{
    const float pwm_on = (float)Settings::getI().GetValueInt32(Settings::Entry::RampOnPercent) / 100.0f;
    const float inc = 0.005f;

    if (is_active) {
        for(float value = 0.0f; value <= 1.0f; value += inc) {
            // Log corrected
            m_sghw_hal->SetRampLight(MISCFA_LinearizeLEDOutput(value)*pwm_on);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    else {
        for(float value = 1.0f; value >= 0.0f; value -= inc) {
            m_sghw_hal->SetRampLight(MISCFA_LinearizeLEDOutput(value)*pwm_on);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

void GateControl::GetState(UIState& ui_state)
{
    xSemaphoreTake(m_semaphore_handle, portMAX_DELAY);
    ui_state.cmd = m_curr_cmd.cmd;
    ui_state.state = m_state_machine.getState();
    // Last error
    ui_state.has_last_error = m_is_in_error;
    ui_state.last_error_code = m_last_error_code;
    strcpy(ui_state.last_error, m_errors);

    strcpy(ui_state.status_text, GetCmdText(m_curr_cmd.cmd));

    ui_state.is_cancel_requested = m_is_cancel_action;
    xSemaphoreGive(m_semaphore_handle);
}

void GateControl::ReleaseClamp()
{
    // Release the clamp
    m_sghw_hal->PowerUpServo();
    m_sghw_hal->SetServo(Settings::getI().GetValueDouble(Settings::Entry::ClampReleasedPWM));
    vTaskDelay(pdMS_TO_TICKS(500));
}

void GateControl::LockClamp()
{
    m_sghw_hal->PowerUpServo();
    m_sghw_hal->SetServo(Settings::getI().GetValueDouble(Settings::Entry::ClampLockedPWM));
    vTaskDelay(pdMS_TO_TICKS(500));
    m_sghw_hal->PowerDownServo();
}