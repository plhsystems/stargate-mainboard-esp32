#ifndef _HWCONFIG_H_
#define _HWCONFIG_H_

#include "driver/gpio.h"

// Motor control
#define HWCONFIG_STEPPER_DIR_PIN GPIO_NUM_33
#define HWCONFIG_STEPPER_STEP_PIN GPIO_NUM_25
#define HWCONFIG_STEPPER_SLP_PIN GPIO_NUM_26

// Hall sensor to detect home position
#define HWCONFIG_HOMESENSOR_PIN GPIO_NUM_32

// Servo-motor
#define HWCONFIG_SERVOMOTOR_PIN GPIO_NUM_18

// Ramp led control
#define HWCONFIG_RAMPLED_PIN GPIO_NUM_23

// External button (input only)
#define HWCONFIG_UIBUTTON_PIN GPIO_NUM_35

// Wormhole LEDs
#define HWCONFIG_WORMHOLELEDS_PIN GPIO_NUM_19
#define HWCONFIG_WORMHOLELEDS_RMTCHANNEL RMT_CHANNEL_0
#define HWCONFIG_WORMHOLELEDS_LEDCOUNT 48

// Sanity led
#define HWCONFIG_SANITY_PIN GPIO_NUM_5


#endif