/*  Ring Firmware - BLE Control

   This code controls the ring LEDs via BLE communication.
*/
#include "fwconfig.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_pm.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "gpio.h"
#include "SGUComm.hpp"
#include "SGURing.hpp"
#include "esp_crc.h"
#include "esp_ota_ops.h"
#include "misc-formula.h"

extern "C" {
#include "ble_server.h"
}

using namespace SGUCommNS;

#define SGU_GATTS_TAG "SGU-GATTS"

// If we go beyond 160 it risk damaging the internal battery.
// 0.035 A x 45 * 160/255
#define LED_OUTPUT_MAX (160)
#define LED_OUTPUT_IDLE (100)

static const char *TAG = "Main";

static volatile bool m_bIsSuicide = false;
static volatile TickType_t m_lAutoOffTicks = 0;
static volatile TickType_t m_ulAutoOffTimeoutMs = FWCONFIG_HOLDPOWER_DELAY_MS;

// Chevron animation
static int32_t m_s32ChevronAnim = -1;

static esp_pm_lock_handle_t m_lockHandle;

extern "C" {
    void app_main();

    static void ResetAutoOffTicks();

    static void SGUBRTurnOffHandler();
    static void SGUBRUpdateLightHandler(const SUpdateLightArg* psArg);
    static void SGUBRChevronsLightningHandler(const SChevronsLightningArg* psChevronLightningArg);
    static void SGUBRGotoFactory();

    // BLE action handlers
    static void BLE_HeartbeatHandler(void);
    static void BLE_AnimationHandler(SGUCommNS::EChevronAnimation animation);
    static void BLE_SymbolsHandler(const uint8_t symbol_bits[6]);
    static void BLE_PowerOffHandler(void);
    static void BLE_LightSymbolHandler(uint8_t symbol_index, uint8_t level_pwm);
    static void BLE_GotoFactoryHandler(void);
}

static const SConfig m_sConfig =
{
    //.fnKeepAliveHandler = SGUBRKeepAliveHandler,
    .fnTurnOffHandler = SGUBRTurnOffHandler,
    .fnUpdateLightHandler = SGUBRUpdateLightHandler,
    .fnChevronsLightningHandler = SGUBRChevronsLightningHandler,
    .fnGotoFactoryHandler = SGUBRGotoFactory,
    .fnPingPongHandler = NULL,
};

static void LedRefreshTask(void *pvParameters)
{
    // First refresh
     // Initialize chevrons dimmed lit
    for(int i = 0; i < HWCONFIG_WS1228B_LEDCOUNT; i++)
    {
        if ((i % 5) == 0)
            GPIO_SetPixel(i, LED_OUTPUT_IDLE, LED_OUTPUT_IDLE, LED_OUTPUT_IDLE);
        else
            GPIO_SetPixel(i, 0, 0, 0);
    }

    while(true)
    {
        // Detect the switch status
        ESP_ERROR_CHECK(esp_pm_lock_acquire(m_lockHandle));
        GPIO_RefreshPixels();

        // Play chevron animation
        if (m_s32ChevronAnim >= 0)
        {
            switch((EChevronAnimation)m_s32ChevronAnim)
            {
                case EChevronAnimation::Chevron_PoweringOff:
                {
                    ESP_LOGI(TAG, "Animation / Chevron_PoweringOff");
                    // Light up the hidden chevron RED.
                    GPIO_SetPixel(SGURingNS::ChevronIndexToLedIndex(5), LED_OUTPUT_MAX, 0, 0);
                    GPIO_SetPixel(SGURingNS::ChevronIndexToLedIndex(6), LED_OUTPUT_MAX, 0, 0);
                    break;
                }
                case EChevronAnimation::Chevron_FadeIn:
                {
                    ESP_LOGI(TAG, "Animation / FadeIn");

                    for(float fltBrightness = 0.0f; fltBrightness <= 1.0f; fltBrightness += 0.05f)
                    {
                        const uint8_t u8Brightness = (uint8_t)(MISCFA_LinearizeLEDOutput(fltBrightness) * LED_OUTPUT_MAX);

                        for(int32_t i = 0; i < HWCONFIG_WS1228B_LEDCOUNT; i++)
                        {
                            if (SGURingNS::IsLEDIndexChevron(i))
                                GPIO_SetPixel(i, u8Brightness, u8Brightness, u8Brightness);
                            else
                                GPIO_SetPixel(i, 0, 0, 0);
                        }
                        GPIO_RefreshPixels();
                        vTaskDelay(pdMS_TO_TICKS(40));
                    }
                    break;
                }
                case EChevronAnimation::Chevron_FadeOut:
                {
                    ESP_LOGI(TAG, "Animation / Chevron_FadeOut");
                    for(float fltBrightness = 1.0f; fltBrightness >= 0.0f; fltBrightness -= 0.05f)
                    {
                        const uint8_t u8Brightness = (fltBrightness < 0.05f ? 0 : (uint8_t)(MISCFA_LinearizeLEDOutput(fltBrightness) * LED_OUTPUT_MAX));

                        for(int32_t i = 0; i < HWCONFIG_WS1228B_LEDCOUNT; i++)
                        {
                            if (SGURingNS::IsLEDIndexChevron(i))
                                GPIO_SetPixel(i, u8Brightness, u8Brightness, u8Brightness);
                            else
                                GPIO_SetPixel(i, 0, 0, 0);
                        }
                        GPIO_RefreshPixels();
                        vTaskDelay(pdMS_TO_TICKS(40));
                    }
                    break;
                }
                case EChevronAnimation::Chevron_ErrorToWhite:
                {
                    ESP_LOGI(TAG, "Animation / Error");
                    for(int32_t i = 0; i < HWCONFIG_WS1228B_LEDCOUNT; i++)
                    {
                        if (SGURingNS::IsLEDIndexChevron(i))
                            GPIO_SetPixel(i, LED_OUTPUT_MAX, 0, 0);
                        else
                            GPIO_SetPixel(i, 0, 0, 0);
                    }
                    GPIO_RefreshPixels();
                    vTaskDelay(pdMS_TO_TICKS(1500));

                    for(float fltBrightness = 0.0f; fltBrightness <= 1.0f; fltBrightness += 0.05f)
                    {
                        const uint8_t u8Brightness = (uint8_t)(MISCFA_LinearizeLEDOutput(fltBrightness) * LED_OUTPUT_MAX);
                        for(int32_t i = 0; i < HWCONFIG_WS1228B_LEDCOUNT; i++)
                        {
                            if (SGURingNS::IsLEDIndexChevron(i))
                                GPIO_SetPixel(i, LED_OUTPUT_MAX, u8Brightness, u8Brightness);
                        }
                        GPIO_RefreshPixels();
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }
                    break;
                }
                case EChevronAnimation::Chevron_ErrorToOff:
                {
                    ESP_LOGI(TAG, "Animation / Error");
                    for(int32_t i = 0; i < HWCONFIG_WS1228B_LEDCOUNT; i++)
                    {
                        if (SGURingNS::IsLEDIndexChevron(i))
                            GPIO_SetPixel(i, LED_OUTPUT_MAX, 0, 0);
                        else
                            GPIO_SetPixel(i, 0, 0, 0);
                    }

                    GPIO_RefreshPixels();
                    vTaskDelay(pdMS_TO_TICKS(1500));

                    for(float fltBrightness = 1.0f; fltBrightness >= 0.0f; fltBrightness -= 0.05f)
                    {
                        const uint8_t u8Brightness = (fltBrightness < 0.05f ? 0 : (uint8_t)(MISCFA_LinearizeLEDOutput(fltBrightness) * LED_OUTPUT_MAX));

                        for(int32_t i = 0; i < HWCONFIG_WS1228B_LEDCOUNT; i++)
                        {
                            if (SGURingNS::IsLEDIndexChevron(i))
                                GPIO_SetPixel(i, u8Brightness, 0, 0);
                        }
                        GPIO_RefreshPixels();
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }
                    break;
                }
                case EChevronAnimation::Chevron_AllSymbolsOn:
                {
                    ESP_LOGI(TAG, "Animation / Chevron_AllSymbolsOn");
                    for(float fltBrightness = 0.0f; fltBrightness <= 1.0f; fltBrightness += 0.05f)
                    {
                        const uint8_t u8Brightness = (uint8_t)(MISCFA_LinearizeLEDOutput(fltBrightness) * LED_OUTPUT_MAX);
                        for(int32_t i = 0; i < HWCONFIG_WS1228B_LEDCOUNT; i++)
                        {
                            if (SGURingNS::IsLEDIndexChevron(i))
                                GPIO_SetPixel(i, u8Brightness, u8Brightness, u8Brightness);
                            else
                                GPIO_SetPixel(i, 5, 5, 5);
                        }
                        GPIO_RefreshPixels();
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }
                    break;
                }
                case EChevronAnimation::Chevron_NoSymbols:
                {
                    ESP_LOGI(TAG, "Animation / Chevron_NoSymbols");
                    for(int32_t i = 0; i < HWCONFIG_WS1228B_LEDCOUNT; i++)
                    {
                        if (SGURingNS::IsLEDIndexChevron(i))
                            GPIO_SetPixel(i, LED_OUTPUT_MAX, LED_OUTPUT_MAX, LED_OUTPUT_MAX);
                        else
                            GPIO_SetPixel(i, 0, 0, 0);
                    }
                    GPIO_RefreshPixels();
                    break;
                }
                default:
                    ESP_LOGE(TAG, "Unknown animation");
                    break;
            }

            m_s32ChevronAnim = -1; // No more animation to process
        }

        GPIO_RefreshPixels();
        ESP_ERROR_CHECK(esp_pm_lock_release(m_lockHandle));
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void ResetAutoOffTicks()
{
    m_lAutoOffTicks = xTaskGetTickCount();
    m_bIsSuicide = false;
    // Hold the power pin, it's likely about to move
    GPIO_EnableHoldPowerPin(true);
}

// static void SGUBRKeepAliveHandler(const SKeepAliveArg* psKeepAliveArg)
// {
//     ESP_LOGI(TAG, "BLE Keep Alive received, resetting timer. Time out set at: %u", /*0*/(uint)psKeepAliveArg->u32MaximumTimeMS);
//     m_ulAutoOffTimeoutMs = psKeepAliveArg->u32MaximumTimeMS + (psKeepAliveArg->u32MaximumTimeMS/2);
//     ResetAutoOffTicks();
// }

static void SGUBRTurnOffHandler()
{
    ESP_LOGI(TAG, "BLE Turn Off received");
    m_bIsSuicide = true;
}

static void SGUBRUpdateLightHandler(const SUpdateLightArg* psArg)
{
    ESP_LOGI(TAG, "BLE Update light received. Lights: %u", /*0*/(unsigned int)psArg->u8LightCount);

     // Keep chevrons dimly lit
    for(int32_t i = 0; i < psArg->u8LightCount; i++)
    {
        uint8_t u8LightIndex = psArg->u8Lights[i];

        if (u8LightIndex >= HWCONFIG_WS1228B_LEDCOUNT)
        {
            ESP_LOGE(TAG, "Invalid light index, %u", /*0*/u8LightIndex);
            continue;
        }

        GPIO_SetPixel(u8LightIndex, psArg->sColor.u8Red, psArg->sColor.u8Green, psArg->sColor.u8Blue);
        // ESP_LOGI(TAG, "Led index change: %d, Red: %d, Green: %d, Blue: %d", u8LightIndex, psArg->rgb.u8Red, psArg->rgb.u8Green, psArg->rgb.u8Blue);
    }

    ResetAutoOffTicks();
}

static void SGUBRChevronsLightningHandler(const SChevronsLightningArg* psChevronLightningArg)
{
    ESP_LOGI(TAG, "BLE EChevron light received");

    // Not ready for chevron animation yet.
    m_s32ChevronAnim = (int32_t)psChevronLightningArg->eChevronAnim;
    ResetAutoOffTicks();
}

static void SGUBRGotoFactory()
{
    ESP_LOGI(TAG, "Goto factory mode");

    const esp_partition_t* p_factory_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);

    if (p_factory_partition == NULL)
    {
        ESP_LOGE(TAG, "Factory partition cannot be found");
        return;
    }

    ESP_LOGI(TAG, "Set boot partition to factory mode");
    esp_ota_set_boot_partition(p_factory_partition);
    esp_restart();

    ResetAutoOffTicks();
}

// BLE Action Handlers

static void BLE_HeartbeatHandler(void)
{
    ESP_LOGI(TAG, "BLE Heartbeat received");
    ResetAutoOffTicks();
}

static void BLE_AnimationHandler(SGUCommNS::EChevronAnimation animation)
{
    ESP_LOGI(TAG, "BLE Animation requested: %u", (unsigned int)animation);
    m_s32ChevronAnim = (int32_t)animation;
    ResetAutoOffTicks();
}

static void BLE_SymbolsHandler(const uint8_t symbol_bits[6])
{
    ESP_LOGI(TAG, "BLE Set symbols: %02X %02X %02X %02X %02X %02X",
             symbol_bits[0], symbol_bits[1], symbol_bits[2],
             symbol_bits[3], symbol_bits[4], symbol_bits[5]);

    // Turn off all symbols first
    for (int32_t i = 0; i < HWCONFIG_WS1228B_LEDCOUNT; i++) {
        if (!SGURingNS::IsLEDIndexChevron(i)) {
            GPIO_SetPixel(i, 0, 0, 0);
        }
    }

    // Light up specified symbols (48 symbols total, 1 bit each)
    for (int32_t symbol_idx = 0; symbol_idx < 48; symbol_idx++) {
        // Check if this symbol bit is set
        uint8_t byte_idx = symbol_idx / 8;
        uint8_t bit_idx = symbol_idx % 8;
        bool is_lit = (symbol_bits[byte_idx] & (1 << bit_idx)) != 0;

        if (is_lit && symbol_idx < HWCONFIG_WS1228B_LEDCOUNT) {
            // Convert symbol index to LED index and light it up
            int32_t led_idx = SGURingNS::SymbolToLedIndex(symbol_idx);
            if (led_idx >= 0 && led_idx < HWCONFIG_WS1228B_LEDCOUNT) {
                GPIO_SetPixel(led_idx, LED_OUTPUT_MAX, LED_OUTPUT_MAX, LED_OUTPUT_MAX);
            }
        }
    }

    ResetAutoOffTicks();
}

static void BLE_PowerOffHandler(void)
{
    ESP_LOGI(TAG, "BLE Power Off received");
    m_bIsSuicide = true;
}

static void BLE_LightSymbolHandler(uint8_t symbol_index, uint8_t level_pwm)
{
    ESP_LOGI(TAG, "BLE Light symbol %u", symbol_index);

    // Convert symbol index to LED index and light it up
    int32_t led_idx = SGURingNS::SymbolToLedIndex(symbol_index);
    if (led_idx >= 0 && led_idx < HWCONFIG_WS1228B_LEDCOUNT) {
        GPIO_SetPixel(led_idx, level_pwm, level_pwm, level_pwm);
    } else {
        ESP_LOGE(TAG, "Invalid symbol index: %u", symbol_index);
    }

    ResetAutoOffTicks();
}

static void BLE_GotoFactoryHandler(void)
{
    ESP_LOGI(TAG, "BLE Goto factory mode");

    const esp_partition_t* p_factory_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);

    if (p_factory_partition == NULL)
    {
        ESP_LOGE(TAG, "Factory partition cannot be found");
        return;
    }

    ESP_LOGI(TAG, "Set boot partition to factory mode");
    esp_ota_set_boot_partition(p_factory_partition);
    esp_restart();

    ResetAutoOffTicks();
}

void app_main(void)
{
    ESP_ERROR_CHECK(esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "NoLightSleep", &m_lockHandle));

    m_lAutoOffTicks = xTaskGetTickCount();

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    GPIO_Init();
    GPIO_EnableHoldPowerPin(true);

    // Initialize BLE server with action callbacks
    ble_action_callbacks_t ble_callbacks = {
        .heartbeat_cb = BLE_HeartbeatHandler,
        .animation_cb = BLE_AnimationHandler,
        .symbols_cb = BLE_SymbolsHandler,
        .poweroff_cb = BLE_PowerOffHandler,
        .light_symbol_cb = BLE_LightSymbolHandler,
        .goto_factory_cb = BLE_GotoFactoryHandler,
    };

    if (ble_server_init(&ble_callbacks) != 0) {
        ESP_LOGE(TAG, "Failed to initialize BLE server");
    }

    xTaskCreatePinnedToCore(&LedRefreshTask, "RefreshLEDs", 4000, NULL, 10, NULL, 0);

    long switchTicks = 0;
    bool last_is_suicide = false;

    while(true)
    {
        const bool switch_state = gpio_get_level((gpio_num_t)FWCONFIG_SWITCH_PIN);
        if (!switch_state) // Up
        {
            if (switchTicks == 0)
                switchTicks = xTaskGetTickCount();

            // If we hold the switch long enough it stop the process.
            if ( (xTaskGetTickCount() - switchTicks) > pdMS_TO_TICKS(FWCONFIG_SWITCH_HOLDDELAY_MS))
            {
                SGUBRGotoFactory();
            }
        }
        else {
            switchTicks = 0; // Reset
        }

        if (!m_bIsSuicide)
        {
            // Kill the power after 10 minutes maximum
            if ((xTaskGetTickCount() - m_lAutoOffTicks) > pdMS_TO_TICKS(m_ulAutoOffTimeoutMs))
            {
                m_bIsSuicide = true;
            }
        }

        // Means cutting to power to itself.
        if (m_bIsSuicide)
        {
            if (!last_is_suicide)
            {
                m_s32ChevronAnim = (int32_t)EChevronAnimation::Chevron_PoweringOff;
                ESP_LOGW(TAG, "Suicide animation");
            }

            // Release the power pin
            // Delay for animation before stop
            GPIO_EnableHoldPowerPin(false);
            // At this point if there are no external power to maintain it, it should die.
            vTaskDelay(pdMS_TO_TICKS(250));
            ESP_LOGW(TAG, "Seems like we will live");
        }

        last_is_suicide = m_bIsSuicide;
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
