#pragma once

#include <cstdint>
#include "../Gate/Chevron.hpp"

enum class MotorDirection
{
    Stop = 0,

    Forward,
    Backward
};

class SGHW_HAL
{
    public:
    SGHW_HAL() { }

    public:
    /*! @brief Initialize all pins and driver. Should be done fast after the MCU startup */
    virtual void Init() { }

    /*! @brief Set chevron lightning status
        @param eChevron     Chevron number
        @param bState       OFF or ON*/
    virtual void SetChevronLight(EChevron eChevron, bool bState) { }

    /*! @brief Ramp lightning output, based on a PWM.
        @param dPerc    PWM value between [0 and 1] */
    virtual void SetRampLight(double dPerc) { };

    /*! @brief EChevron motor control, each chevron have one motor.
        @param eChevron     Chevron number
        @param dPerc        PWM value between [0 and 1] */
    virtual void MoveChevronMotor(EChevron eChevron, MotorDirection motorDir) { }

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
        @param dPosition    New position between [0, 1] */
    virtual void SetServo(double dPosition) { }
    /*! @brief Power down the servo motor, it could be noisy while powered up.. */
    virtual void PowerDownServo() { }

    // Wormhole related
    /*! @brief Get wormhole neopixel led strip count. */
    virtual int32_t GetWHPixelCount() { return 0; }

    /*! @brief Set neopixel color on the led strip, by index.
        @param u32Index LED index
        @param u8Red    Red channel [0, 255]
        @param u8Green  Green channel [0, 255]
        @param u8Blue   Blue channel [0, 255] */
    virtual void SetWHPixel(uint32_t u32Index, uint8_t u8Red, uint8_t u8Green, uint8_t u8Blue) { }

    /*! @brief Clear all wormhole led strip pixels. */
    virtual void ClearAllWHPixels() { }
    /*! @brief Refresh the wormhole led strip. */
    virtual void RefreshWHPixels() { }

    /*! @brief Set sanity the LED status.
        @param bState   false: light is off, true: light is on */
    virtual void SetSanityLED(bool bState) { }

    /*! @brief Is home sensor active ? Meaning the magnet near the home point. */
    virtual bool GetIsHomeSensorActive() { return false; }
};
