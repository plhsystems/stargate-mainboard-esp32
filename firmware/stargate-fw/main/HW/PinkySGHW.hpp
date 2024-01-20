#pragma once

#include <cstdint>
#include "SGHW_HAL.hpp"
#include "led_strip.h"

class PinkySGHW : public SGHW_HAL
{
    public:
    PinkySGHW();

    void Init() override;

    // Ramp light
    void SetRampLight(double dPerc);

    void PowerUpStepper() override;
    void MoveStepper(int32_t s32StepCount) override;
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
    led_strip_handle_t led_strip;

    double m_dLastServoPosition;
};