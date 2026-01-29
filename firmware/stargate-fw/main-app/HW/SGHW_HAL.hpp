#pragma once

#include <cstdint>
#include "../Gate/Chevron.hpp"

enum class MotorDirection
{
    Stop = 0,

    Forward,
    Backward
};

enum class ETransition
{
    Rising,
    Failing
};

enum class ESpinDirection
{
    CCW,
    CW
};

class SGHW_HAL
{
    public:
    SGHW_HAL() { }

    public:
    /*! @brief Initialize all pins and driver. Should be done fast after the MCU startup */
    virtual void Init() { }

    /*! @brief Set chevron lightning status
        @param chevron     Chevron number
        @param state       OFF or ON*/
    virtual void SetChevronLight(EChevron chevron, bool state) { }

    /*! @brief Ramp lightning output, based on a PWM.
        @param perc    PWM value between [0 and 1] */
    virtual void SetRampLight(double perc) { };

    /*! @brief EChevron motor control, each chevron have one motor.
        @param chevron     Chevron number
        @param perc        PWM value between [0 and 1] */
    virtual void MovchevronMotor(EChevron chevron, MotorDirection motor_dir) { }

    // Stepper.
    /*! @brief Active the power on the stepper driver and motor. */
    virtual void PowerUpStepper() { }
    /*! @brief Move the stepper, basically if you stand in front of the gate the ring will spin clockwise.  */
    virtual void StepStepperCW() { }
    /*! @brief Move the stepper, basically if you stand in front of the gate the ring will spin counter-clockwise.  */
    virtual void StepStepperCCW() { }
    /*! @brief Power down the stepper driver. It helps to reduce motor heating when on IDLE. */
    virtual void PowerDownStepper() { }

    // Servo motor
    /*! @brief Active the power on the servo motor. */
    virtual void PowerUpServo() { }
    /*! @brief Change the servo position.
        @param position    New position between [0, 1] */
    virtual void SetServo(double position) { }
    /*! @brief Power down the servo motor, it could be noisy while powered up.. */
    virtual void PowerDownServo() { }

    // Wormhole related
    /*! @brief Get wormhole neopixel led strip count. */
    virtual int32_t GetWHPixelCount() { return 0; }

    /*! @brief Set neopixel color on the led strip, by index.
        @param index LED index
        @param red    Red channel [0, 255]
        @param green  Green channel [0, 255]
        @param blue   Blue channel [0, 255] */
    virtual void SetWHPixel(uint32_t index, uint8_t red, uint8_t green, uint8_t blue) { }

    /*! @brief Clear all wormhole led strip pixels. */
    virtual void ClearAllWHPixels() { }
    /*! @brief Refresh the wormhole led strip. */
    virtual void RefreshWHPixels() { }

    /*! @brief Set sanity the LED status.
        @param state   false: light is off, true: light is on */
    virtual void SetSanityLED(bool state) { }

    /*! @brief Is home sensor active ? Meaning the magnet near the home point. */
    virtual bool GetIsHomeSensorActive() { return false; }

    // Stepper
    virtual bool SpinUntil(ESpinDirection spin_direction, ETransition transition, uint32_t timeout_ms, int32_t* ref_tick_count) { return false;};

    virtual bool MoveStepperTo(int32_t ticks, uint32_t timeout_ms) { return false;};
};
