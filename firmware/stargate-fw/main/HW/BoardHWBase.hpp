#pragma once

#include <cstdint>
#include "../Common/Chevron.hpp"
#include "HWConfig.hpp"
#include "HWBase.hpp"

class BoardHWBase
{
    public:
    BoardHWBase();

    public:
    virtual void Init();

    // Chevron light
    virtual void SetChevronLight(Chevron eChevron, bool bState);
    // Ramp light
    virtual void SetRampLight(double dPerc);
    // Home sensor
    virtual bool GetIsHomeSensorActive();

    // Chevron Motor
    virtual void MoveChevronMotor(Chevron eChevron, MotorDirection motorDir);

    // Stepper.
    virtual void PowerUpStepper();
    virtual void MoveStepper(int32_t s32StepCount);
    virtual void PowerDownStepper();

    // Servo motor
    virtual void PowerUpServo();
    virtual void SetServo(double dPosition);
    virtual void PowerDownServo();

    // Wormhole related
    virtual int32_t GetWHPixelCount();
    virtual void SetWHPixel(uint32_t u32Index, uint8_t u8Red, uint8_t u8Green, uint8_t u8Blue);
    virtual void ClearAllWHPixels();
    virtual void RefreshWHPixels();
};
