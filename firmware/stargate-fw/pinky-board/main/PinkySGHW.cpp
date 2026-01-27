#include "PinkySGHW.hpp"
#include <stdexcept>
#include "led_strip.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"
#include "driver/ledc.h"
#include "freertos/task.h"
#include "misc-macro.h"

#define TAG "PinkySGHW"

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
    : m_last_servo_position(0)
{
}

void PinkySGHW::Init()
{
    m_mutex_handle = xSemaphoreCreateMutexStatic( &m_mutex_buffer );

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

    // Servo PIN - Initialize MCPWM for servo control using new API
    // Create timer (20ms period for 50Hz servo signal)
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,  // 1MHz, 1us per tick
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = 20000,  // 20ms period (50Hz)
        .flags = {}
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &m_servo.timer));

    // Create operator
    mcpwm_operator_config_t operator_config = {
        .group_id = 0,  // Same group as timer
        .flags = {}
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &m_servo.oper));

    // Connect operator to timer
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(m_servo.oper, m_servo.timer));

    // Create comparator
    mcpwm_comparator_config_t comparator_config = {
        .flags = {
            .update_cmp_on_tez = true,
        }
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(m_servo.oper, &comparator_config, &m_servo.comparator));

    // Create generator
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = SERVOMOTOR_PIN,
        .flags = {}
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(m_servo.oper, &generator_config, &m_servo.generator));

    // Set generator actions
    // On timer empty (start of period), set output high
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(m_servo.generator,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // On compare match, set output low
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(m_servo.generator,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, m_servo.comparator, MCPWM_GEN_ACTION_LOW)));

    // Enable and start timer
    ESP_ERROR_CHECK(mcpwm_timer_enable(m_servo.timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(m_servo.timer, MCPWM_TIMER_START_NO_STOP));

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
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // different clock source can lead to different power consumption
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .mem_block_symbols = 128,
        .flags = { .with_dma = false } // whether to enable the DMA feature
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);


    // Stepper control timer
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &tmr_signal_callback,
        .arg = this,
        .dispatch_method = ESP_TIMER_ISR,
        .name = "stepper_timer",
    };

    /* Start the timers */
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &this->m_stepper.signal_timer_handle));
}

void PinkySGHW::SetChevronLight(EChevron chevron, bool state)
{
    // No such things on the pinky board.
}

void PinkySGHW::SetRampLight(double dPerc)
{
    LockMutex();
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4095 * dPerc));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
    UnlockMutex();
}

void PinkySGHW::PowerUpStepper()
{
    gpio_set_level(STEPPER_SLP_PIN, true);
}

void PinkySGHW::StepStepperCW()
{
    gpio_set_level(STEPPER_DIR_PIN, true);
    gpio_set_level(STEPPER_STEP_PIN, true);
    esp_rom_delay_us(4);
    gpio_set_level(STEPPER_STEP_PIN, false);
    esp_rom_delay_us(4);
}

void PinkySGHW::StepStepperCCW()
{
    gpio_set_level(STEPPER_DIR_PIN, false);
    gpio_set_level(STEPPER_STEP_PIN, true);
    esp_rom_delay_us(4);
    gpio_set_level(STEPPER_STEP_PIN, false);
    esp_rom_delay_us(4);
}

void PinkySGHW::PowerDownStepper()
{
    gpio_set_level(STEPPER_SLP_PIN, false);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void PinkySGHW::PowerUpServo()
{
    SetServo(m_last_servo_position);
}

void PinkySGHW::SetServo(double dPosition)
{
    LockMutex();
    // Convert position (0.0 to 1.0) to pulse width
    // Servo typically needs 1000us (1ms) to 2000us (2ms) pulse width
    // Center is at 1500us (1.5ms)
    const uint32_t pulse_width_us = 1000 + (uint32_t)(dPosition * 1000);
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(m_servo.comparator, pulse_width_us));
    m_last_servo_position = dPosition;
    ESP_LOGI(TAG, "pulse_width_us: %lu pos: %f", pulse_width_us, (float)dPosition);
    UnlockMutex();
}

void PinkySGHW::PowerDownServo()
{
    LockMutex();
    // Set comparator to 0 to keep output low
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(m_servo.comparator, 0));
    UnlockMutex();
}

// Wormhole related
int32_t PinkySGHW::GetWHPixelCount()
{
    return WORMHOLELEDS_LEDCOUNT;
}

void PinkySGHW::SetWHPixel(uint32_t index, uint8_t red, uint8_t green, uint8_t blue)
{
    LockMutex();
    led_strip_set_pixel(led_strip, index, red, green, blue);
    UnlockMutex();
}

void PinkySGHW::ClearAllWHPixels()
{
    LockMutex();
    led_strip_clear(led_strip);
    UnlockMutex();
}

void PinkySGHW::RefreshWHPixels()
{
    LockMutex();
    led_strip_refresh(led_strip);
    UnlockMutex();
}

void PinkySGHW::SetSanityLED(bool state)
{
    // The sanity LED is ground driven.
    gpio_set_level(SANITY_PIN, !state);
}

bool PinkySGHW::GetIsHomeSensorActive()
{
    return !gpio_get_level(HOMESENSOR_PIN);
}

void PinkySGHW::SpinUntil(ESpinDirection spin_direction, ETransition transition, uint32_t timeout_ms, int32_t* ref_tick_count)
{
    TickType_t ttStart = xTaskGetTickCount();
    bool old_sensor_state = GetIsHomeSensorActive();

    while ((xTaskGetTickCount() - ttStart) < pdMS_TO_TICKS(timeout_ms))
    {
        /*if (m_bIsCancelAction) {
            throw std::runtime_error("Cancelled by the user");
        }*/

        const bool new_home_sensor_state = GetIsHomeSensorActive();

        if (spin_direction == ESpinDirection::CCW) {
            StepStepperCCW();
            if (ref_tick_count != nullptr) {
                (*ref_tick_count)++;
            }
        }
        if (spin_direction == ESpinDirection::CW) {
            StepStepperCW();
            if (ref_tick_count != nullptr) {
                (*ref_tick_count)--;
            }
        }

        const bool is_rising = !old_sensor_state && new_home_sensor_state;
        const bool is_falling = old_sensor_state && !new_home_sensor_state;
        if (ETransition::Rising == transition && is_rising)
        {
            ESP_LOGI(TAG, "Rising transition detected");
            return;
        }
        else if (ETransition::Failing == transition && is_falling) {
            ESP_LOGI(TAG, "Failing transition detected");
            return;
        }

        old_sensor_state = new_home_sensor_state;
        vTaskDelay(1);
    }

    throw std::runtime_error("Unable to complete the spin operation");
}

void PinkySGHW::MoveStepperTo(int32_t ticks, uint32_t timeout_ms)
{
    // Setup the parameters
    this->m_stepper.task_control_handle = xTaskGetCurrentTaskHandle();
    this->m_stepper.count = 0;
    this->m_stepper.target = abs(ticks);
    this->m_stepper.period = 1;
    this->m_stepper.is_ccw = ticks > 0;

    ESP_ERROR_CHECK(esp_timer_start_once(this->m_stepper.signal_timer_handle, this->m_stepper.period));

    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( timeout_ms );

    /* Wait to be notified of an interrupt. */
    uint32_t ulNotifiedValue = 0;
    const BaseType_t xResult = xTaskNotifyWait(
        pdFALSE,          /* Don't clear bits on entry. */
        ULONG_MAX,        /* Clear all bits on exit. */
        &ulNotifiedValue, /* Stores the notified value. */
        xMaxBlockTime );

    // No longer need to run the timer ...
    esp_timer_stop(this->m_stepper.signal_timer_handle);

    if( xResult != pdPASS )
    {
        throw std::runtime_error("Error, cannot reach it's destination with-in time ...");
    }
}

IRAM_ATTR void PinkySGHW::tmr_signal_callback(void* arg)
{
    PinkySGHW* gc = (PinkySGHW*)arg;
    Stepper* step = (Stepper*)&gc->m_stepper;

    static BaseType_t xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;

    const int32_t s32 = MISCMACRO_MIN(abs(step->count) , abs(step->target - step->count));
    /* I just did some tests until I was satisfied */
    /* #1 (100, 1000), (1400, 300)
        a = -0.53846153846153846153846153846154
        b = 1053.84 */
    /* #2 (100, 700), (1400, 200)
        a = -0.3846
        b = 738.44 */
    const int32_t a = -400;
    const int32_t b = 1400000;
    step->period = (a * s32 + b)/1000;

    // I hoped it would reduce jitter.
    step->period = (step->period / 50) * 50;

    if (step->period < 600)
        step->period = 600;
    if (step->period > 1600)
        step->period = 1600;

    // Wait until the period go to low before considering it finished
    if (step->target == step->count)
    {
        xTaskNotifyFromISR( step->task_control_handle,
            STEPEND_BIT,
            eSetBits,
            &xHigherPriorityTaskWoken );
    }
    else {
        // Count every two
        if (step->is_ccw)
            gc->StepStepperCCW();
        else
            gc->StepStepperCW();

        step->count++;

        ESP_ERROR_CHECK(esp_timer_start_once(step->signal_timer_handle, step->period));
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
