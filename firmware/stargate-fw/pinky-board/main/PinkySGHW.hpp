#pragma once

#include <cstdint>
#include "HW/SGHW_HAL.hpp"
#include "led_strip.h"
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "driver/mcpwm_prelude.h"

class PinkySGHW : public SGHW_HAL
{

    #define STEPEND_BIT    0x01
    struct Stepper
    {
        esp_timer_handle_t signal_timer_handle;
        TaskHandle_t task_control_handle;

        int32_t period = 0;
        // Counter
        bool is_ccw;
        int32_t count = 0;
        int32_t target = 0;
    };

    struct ServoControl
    {
        mcpwm_timer_handle_t timer;
        mcpwm_oper_handle_t oper;
        mcpwm_cmpr_handle_t comparator;
        mcpwm_gen_handle_t generator;
    };

    public:
    PinkySGHW();

    void Init() override;

    void SetChevronLight(EChevron chevron, bool state) override;

    // Ramp light
    void SetRampLight(double perc) override;

    void PowerUpStepper() override;
    void StepStepperCW() override;
    void StepStepperCCW() override;
    void PowerDownStepper() override;

    void PowerUpServo() override;
    void SetServo(double position) override;
    void PowerDownServo() override;

    // Wormhole related
    int32_t GetWHPixelCount() override;
    void SetWHPixel(uint32_t index, uint8_t red, uint8_t green, uint8_t blue) override;
    void ClearAllWHPixels() override;
    bool RefreshWHPixels() override;

    void SetSanityLED(bool state);

    bool GetIsHomeSensorActive() override;

    void SendMp3PlayerCMD(const char* cmd) override;

    // Stepper
    bool SpinUntil(ESpinDirection spin_direction, ETransition transition, uint32_t timeout_ms, int32_t* ref_tick_count) override;
    bool MoveStepperTo(int32_t ticks, uint32_t timeout_ms) override;

    private:
    bool LockMutex() { return (pdTRUE == xSemaphoreTake( m_mutex_handle, ( TickType_t ) pdMS_TO_TICKS(100) )); }
    void UnlockMutex() { xSemaphoreGive( m_mutex_handle ); }

    static void tmr_signal_callback(void* arg);

    private:
    Stepper m_stepper;
    ServoControl m_servo;

    led_strip_handle_t led_strip;

    double m_last_servo_position;

    // Mutex
    StaticSemaphore_t m_mutex_buffer; // Define the buffer for the mutex's data structure
    SemaphoreHandle_t m_mutex_handle; // Declare a handle for the mutex
};