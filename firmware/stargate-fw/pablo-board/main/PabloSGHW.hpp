#pragma once

#include <cstdint>
#include "HW/SGHW_HAL.hpp"
#include "led_strip.h"
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"

class PabloSGHW : public SGHW_HAL
{
    public:
    PabloSGHW();

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

    private:
    bool LockMutex() { return (pdTRUE == xSemaphoreTake( m_xMutexHandle, ( TickType_t ) pdMS_TO_TICKS(100) )); }
    void UnlockMutex() { xSemaphoreGive( m_xMutexHandle ); }

    private:
    led_strip_handle_t led_strip;

    double m_last_servo_position;

    // Mutex
    StaticSemaphore_t m_xMutexBuffer; // Define the buffer for the mutex's data structure
    SemaphoreHandle_t m_xMutexHandle; // Declare a handle for the mutex
};