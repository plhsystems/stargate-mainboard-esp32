#pragma once

#include <cstdint>
#include "HW/SGHW_HAL.hpp"
#include "led_strip.h"
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"

class PinkySGHW : public SGHW_HAL
{
    
    #define STEPEND_BIT    0x01
    struct Stepper
    {
        esp_timer_handle_t sSignalTimerHandle;
        TaskHandle_t sTskControlHandle;

        int32_t s32Period = 0;
        // Counter
        bool bIsCCW;
        int32_t s32Count = 0;
        int32_t s32Target = 0;
    };

    public:
    PinkySGHW();

    void Init() override;

    void SetChevronLight(EChevron eChevron, bool bState) override;

    // Ramp light
    void SetRampLight(double dPerc) override;

    void PowerUpStepper() override;
    void StepStepperCW() override;
    void StepStepperCCW() override;
    void PowerDownStepper() override;

    void PowerUpServo() override;
    void SetServo(double dPosition) override;
    void PowerDownServo() override;

    // Wormhole related
    int32_t GetWHPixelCount() override;
    void SetWHPixel(uint32_t u32Index, uint8_t u8Red, uint8_t u8Green, uint8_t u8Blue) override;
    void ClearAllWHPixels() override;
    void RefreshWHPixels() override;

    void SetSanityLED(bool bState);

    bool GetIsHomeSensorActive() override;

    // Stepper
    void SpinUntil(ESpinDirection eSpinDirection, ETransition eTransition, uint32_t u32TimeoutMS, int32_t* ps32refTickCount) override;
    void MoveStepperTo(int32_t s32Ticks, uint32_t u32TimeoutMS) override;

    private:
    bool LockMutex() { return (pdTRUE == xSemaphoreTake( m_xMutexHandle, ( TickType_t ) pdMS_TO_TICKS(100) )); }
    void UnlockMutex() { xSemaphoreGive( m_xMutexHandle ); }

    static void tmr_signal_callback(void* arg);

    private:
    Stepper m_stepper;

    led_strip_handle_t led_strip;

    double m_dLastServoPosition;

    // Mutex
    StaticSemaphore_t m_xMutexBuffer; // Define the buffer for the mutex's data structure
    SemaphoreHandle_t m_xMutexHandle; // Declare a handle for the mutex
};