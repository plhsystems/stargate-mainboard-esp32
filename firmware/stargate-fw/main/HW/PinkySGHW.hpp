#pragma once

#include <cstdint>
#include "SGHW_HAL.hpp"
#include "led_strip.h"
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"

class PinkySGHW : public SGHW_HAL
{
    public:
    PinkySGHW();

    void Init() override;

    // Ramp light
    void SetRampLight(double dPerc);

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

    private:
    bool LockMutex() { return (pdTRUE == xSemaphoreTake( m_xMutexHandle, ( TickType_t ) pdMS_TO_TICKS(100) )); }
    void UnlockMutex() { xSemaphoreGive( m_xMutexHandle ); }

    private:
    led_strip_handle_t led_strip;

    double m_dLastServoPosition;

    // Mutex
    StaticSemaphore_t m_xMutexBuffer; // Define the buffer for the mutex's data structure
    SemaphoreHandle_t m_xMutexHandle; // Declare a handle for the mutex
};