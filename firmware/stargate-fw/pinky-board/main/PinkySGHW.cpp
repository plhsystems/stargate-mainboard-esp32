#include "PinkySGHW.hpp"
#include <stdexcept>
#include "led_strip.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
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
    : m_dLastServoPosition(0)
{
}

void PinkySGHW::Init()
{
    m_xMutexHandle = xSemaphoreCreateMutexStatic( &m_xMutexBuffer );

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
        .mem_block_symbols = 128
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
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &this->m_stepper.sSignalTimerHandle));
}

void PinkySGHW::SetChevronLight(EChevron eChevron, bool bState)
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
    SetServo(m_dLastServoPosition);
}

void PinkySGHW::SetServo(double dPosition)
{
    LockMutex();
    mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, SERVO_PWM2PERCENT(dPosition));
    mcpwm_set_duty_type(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    m_dLastServoPosition = dPosition;
    UnlockMutex();
}

void PinkySGHW::PowerDownServo()
{
    LockMutex();
    mcpwm_set_signal_low(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A);
    UnlockMutex();
}

// Wormhole related
int32_t PinkySGHW::GetWHPixelCount()
{
    return WORMHOLELEDS_LEDCOUNT;
}

void PinkySGHW::SetWHPixel(uint32_t u32Index, uint8_t u8Red, uint8_t u8Green, uint8_t u8Blue)
{
    LockMutex();
    led_strip_set_pixel(led_strip, u32Index, u8Red, u8Green, u8Blue);
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

void PinkySGHW::SetSanityLED(bool bState)
{
    // The sanity LED is ground driven.
    gpio_set_level(SANITY_PIN, !bState);
}

bool PinkySGHW::GetIsHomeSensorActive()
{
    return !gpio_get_level(HOMESENSOR_PIN);
}

void PinkySGHW::SpinUntil(ESpinDirection eSpinDirection, ETransition eTransition, uint32_t u32TimeoutMS, int32_t* ps32refTickCount)
{
    TickType_t ttStart = xTaskGetTickCount();
    bool bOldSensorState = GetIsHomeSensorActive();

    while ((xTaskGetTickCount() - ttStart) < pdMS_TO_TICKS(u32TimeoutMS))
    {
        /*if (m_bIsCancelAction) {
            throw std::runtime_error("Cancelled by the user");
        }*/

        const bool bNewHomeSensorState = GetIsHomeSensorActive();

        if (eSpinDirection == ESpinDirection::CCW) {
            StepStepperCCW();
            if (ps32refTickCount != nullptr) {
                (*ps32refTickCount)++;
            }
        }
        if (eSpinDirection == ESpinDirection::CW) {
            StepStepperCW();
            if (ps32refTickCount != nullptr) {
                (*ps32refTickCount)--;
            }
        }

        const bool bIsRising = !bOldSensorState && bNewHomeSensorState;
        const bool bIsFalling = bOldSensorState && !bNewHomeSensorState;
        if (ETransition::Rising == eTransition && bIsRising)
        {
            ESP_LOGI(TAG, "Rising transition detected");
            return;
        }
        else if (ETransition::Failing == eTransition && bIsFalling) {
            ESP_LOGI(TAG, "Failing transition detected");
            return;
        }

        bOldSensorState = bNewHomeSensorState;
        vTaskDelay(1);
    }

    throw std::runtime_error("Unable to complete the spin operation");
}

void PinkySGHW::MoveStepperTo(int32_t s32Ticks, uint32_t u32TimeoutMS)
{
    // Setup the parameters
    this->m_stepper.sTskControlHandle = xTaskGetCurrentTaskHandle();
    this->m_stepper.s32Count = 0;
    this->m_stepper.s32Target = abs(s32Ticks);
    this->m_stepper.s32Period = 1;
    this->m_stepper.bIsCCW = s32Ticks > 0;

    ESP_ERROR_CHECK(esp_timer_start_once(this->m_stepper.sSignalTimerHandle, this->m_stepper.s32Period));

    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( u32TimeoutMS );

    /* Wait to be notified of an interrupt. */
    uint32_t ulNotifiedValue = 0;
    const BaseType_t xResult = xTaskNotifyWait(pdFALSE,    /* Don't clear bits on entry. */
                        ULONG_MAX,        /* Clear all bits on exit. */
                        &ulNotifiedValue, /* Stores the notified value. */
                        xMaxBlockTime );

    if( xResult != pdPASS )
    {
        esp_timer_stop(this->m_stepper.sSignalTimerHandle);
        throw std::runtime_error("Error, cannot reach it's destination with-in time ...");
    }
}

IRAM_ATTR void PinkySGHW::tmr_signal_callback(void* arg)
{
    PinkySGHW* gc = (PinkySGHW*)arg;
    Stepper* step = (Stepper*)&gc->m_stepper;

    static BaseType_t xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;

    const int32_t s32 = MISCMACRO_MIN(abs(step->s32Count) , abs(step->s32Target - step->s32Count));
    /* I just did some tests until I was satisfied */
    /* #1 (100, 1000), (1400, 300)
        a = -0.53846153846153846153846153846154
        b = 1053.84 */
    /* #2 (100, 700), (1400, 200)
        a = -0.3846
        b = 738.44 */
    const int32_t a = -400;
    const int32_t b = 1400000;
    step->s32Period = (a * s32 + b)/1000;

    // I hoped it would reduce jitter.
    step->s32Period = (step->s32Period / 50) * 50;

    if (step->s32Period < 600)
        step->s32Period = 600;
    if (step->s32Period > 1600)
        step->s32Period = 1600;

    // Wait until the period go to low before considering it finished
    if (step->s32Target == step->s32Count)
    {
        xTaskNotifyFromISR( step->sTskControlHandle,
            STEPEND_BIT,
            eSetBits,
            &xHigherPriorityTaskWoken );
    }
    else {
        // Count every two
        if (step->bIsCCW)
            gc->StepStepperCCW();
        else
            gc->StepStepperCW();

        step->s32Count++;

        ESP_ERROR_CHECK(esp_timer_start_once(step->sSignalTimerHandle, step->s32Period));
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
