#include "PinkySGHW.hpp"
#include "led_strip.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/ledc.h"

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

#define SERVO_PWM2PERCENT(x) ((float)x/20000.0f*100.0f)
#define SERVO_PERCENT2PWM(x) ((float)x/100.0f*20000.0f)

PinkySGHW::PinkySGHW()
    : m_dLastServoPosition(0)
{
}

void PinkySGHW::Init()
{
    //install gpio isr service
    gpio_install_isr_service(0);

    // Sanity leds
    gpio_set_direction(WORMHOLELEDS_PIN, GPIO_MODE_OUTPUT);

    // Sanity leds
    gpio_set_direction(SANITY_PIN, GPIO_MODE_OUTPUT);

    // Ramp LEDs
    gpio_set_direction(RAMPLED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RAMPLED_PIN, false);

    // Stepper PINs
    gpio_set_direction(STEPPER_DIR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(STEPPER_STEP_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(STEPPER_SLP_PIN, GPIO_MODE_OUTPUT);
    PowerDownStepper();

    // Servo PIN
    gpio_set_direction(SERVOMOTOR_PIN, GPIO_MODE_OUTPUT);
    // Initialize motor driver
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, SERVOMOTOR_PIN);
    mcpwm_config_t servo_config;
    servo_config.frequency = 50;  // Frequency = 1000Hz,
    servo_config.cmpr_a = 0;      // Duty cycle of PWMxA = 0
    servo_config.cmpr_b = 0;      // Duty cycle of PWMxb = 0
    servo_config.counter_mode = MCPWM_UP_COUNTER;
    servo_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &servo_config);    // Configure PWM0A & PWM0B with above settings
    // 2/20 ms * 0.5*0.5
    PowerDownServo(); // Don't piss off the servo motor


    // Init ramp LED
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_12_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 5000,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = RAMPLED_PIN,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    SetRampLight(0);

    // Initialize GPIO for home sensor
    gpio_set_direction(HOMESENSOR_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(HOMESENSOR_PIN);

    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = WORMHOLELEDS_PIN,
        .max_leds = WORMHOLELEDS_LEDCOUNT, // sanity LED + at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void PinkySGHW::SetRampLight(double dPerc)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4095 * dPerc));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

void PinkySGHW::PowerUpStepper()
{
    gpio_set_level(STEPPER_SLP_PIN, true);
}

void PinkySGHW::StepStepperCW()
{
    gpio_set_level(STEPPER_DIR_PIN, true);
    gpio_set_level(STEPPER_STEP_PIN, true);
    esp_rom_delay_us(10);
    gpio_set_level(STEPPER_STEP_PIN, false);
    esp_rom_delay_us(10);
}

void PinkySGHW::StepStepperCCW()
{
    gpio_set_level(STEPPER_DIR_PIN, false);
    gpio_set_level(STEPPER_STEP_PIN, true);
    esp_rom_delay_us(10);
    gpio_set_level(STEPPER_STEP_PIN, false);
    esp_rom_delay_us(10);
}

void PinkySGHW::PowerDownStepper()
{
    gpio_set_level(STEPPER_SLP_PIN, false);
}

void PinkySGHW::PowerUpServo()
{
    SetServo(m_dLastServoPosition);
}

void PinkySGHW::SetServo(double dPosition)
{
    mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, SERVO_PWM2PERCENT(dPosition));
    mcpwm_set_duty_type(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    m_dLastServoPosition = dPosition;
}

void PinkySGHW::PowerDownServo()
{
    mcpwm_set_signal_low(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A);
}

// Wormhole related
int32_t PinkySGHW::GetWHPixelCount()
{
    return WORMHOLELEDS_LEDCOUNT;
}

void PinkySGHW::SetWHPixel(uint32_t u32Index, uint8_t u8Red, uint8_t u8Green, uint8_t u8Blue)
{
    led_strip_set_pixel(led_strip, u32Index, u8Red, u8Green, u8Blue);
}

void PinkySGHW::ClearAllWHPixels()
{
    led_strip_clear(led_strip);
}

void PinkySGHW::RefreshWHPixels()
{
    led_strip_refresh(led_strip);
}

void PinkySGHW::SetSanityLED(bool bState)
{
    // The sanity LED is ground driven.
    gpio_set_level(SANITY_PIN, !bState);
}

bool PinkySGHW::GetIsHomeSensorActive()
{
    return !gpio_get_level(HOMESENSOR_PIN);
}