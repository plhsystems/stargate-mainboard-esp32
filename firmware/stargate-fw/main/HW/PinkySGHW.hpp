#pragma once

#include <cstdint>
#include "SGHW_HAL.hpp"
#include "led_strip.h"

class PinkySGHW : public SGHW_HAL
{
    // Motor control
    #define STEPPER_DIR_PIN GPIO_NUM_33
    #define STEPPER_STEP_PIN GPIO_NUM_25
    #define STEPPER_SLP_PIN GPIO_NUM_26

    // Hall sensor to detect home position
    #define HOMESENSOR_PIN GPIO_NUM_32

    // Servo-motor
    #define SERVOMOTOR_PIN GPIO_NUM_18

    // Ramp led control
    #define RAMPLED_PIN GPIO_NUM_23

    // External button (input only)
    #define UIBUTTON_PIN GPIO_NUM_35

    // Wormhole LEDs
    #define WORMHOLELEDS_PIN GPIO_NUM_19
    #define WORMHOLELEDS_RMTCHANNEL RMT_CHANNEL_0
    #define WORMHOLELEDS_LEDCOUNT 48

    // Sanity led
    #define SANITY_PIN GPIO_NUM_5
    public:
    PinkySGHW();
    private:
    led_strip_handle_t led_strip;
};