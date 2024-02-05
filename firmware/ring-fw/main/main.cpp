/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "FWConfig.h"
#include <string.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_pm.h"
#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/ip4_addr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "gpio.h"
#include "SGUComm.hpp"
#include "SGURing.hpp"
#include "esp_now.h"
#include "esp_crc.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_ota_ops.h"
#include "misc-formula.h"

using namespace SGUCommNS;

#define SGU_GATTS_TAG "SGU-GATTS"

#define LED_OUTPUT_MAX (160)
#define LED_OUTPUT_IDLE (100)

static void InitESPNOW();

static const char *TAG = "Main";

static volatile bool m_bIsSuicide = false;
static volatile TickType_t m_lAutoOffTicks = 0;
static volatile TickType_t m_ulAutoOffTimeoutMs = FWCONFIG_HOLDPOWER_DELAY_MS;

// Chevron animation
static volatile int32_t m_s32ChevronAnim = -1;

static esp_pm_lock_handle_t m_lockHandle;

static int m_UdpCommSocket = -1;
static struct sockaddr_storage m_source_addr; // Large enough for both IPv4 or IPv6

extern "C" {
    void app_main();

    static void ResetAutoOffTicks();

    static void SGUBRTurnOffHandler();
    static void SGUBRUpdateLightHandler(const SUpdateLightArg* psArg);
    static void SGUBRChevronsLightningHandler(const SChevronsLightningArg* psChevronLightningArg);
    static void SGUBRGotoFactory();
    static void PingHandler(const SPingPongArg* psArg);
}

static const SConfig m_sConfig =
{
    //.fnKeepAliveHandler = SGUBRKeepAliveHandler,
    .fnTurnOffHandler = SGUBRTurnOffHandler,
    .fnUpdateLightHandler = SGUBRUpdateLightHandler,
    .fnChevronsLightningHandler = SGUBRChevronsLightningHandler,
    .fnGotoFactoryHandler = SGUBRGotoFactory,
    .fnPingHandler = PingHandler,
};

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void wifistation_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

static void InitESPNOW()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );

    // Soft Access Point Mode
    esp_netif_t* wifiSTA = esp_netif_create_default_wifi_sta();

    esp_netif_ip_info_t ipInfoSTA;
    IP4_ADDR(&ipInfoSTA.ip, 192, 168, 66, 250);
	IP4_ADDR(&ipInfoSTA.gw, 192, 168, 66, 250);
	IP4_ADDR(&ipInfoSTA.netmask, 255, 255, 255, 0);
    esp_netif_dhcpc_stop(wifiSTA);
	esp_netif_set_ip_info(wifiSTA, &ipInfoSTA);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifistation_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifistation_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_configSTA = {
        .sta = {
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    strcpy((char*)wifi_configSTA.sta.ssid, FWCONFIG_MASTERBASE_SSID);

    if (strlen(FWCONFIG_MASTERBASE_PASS) == 0)
    {
        wifi_configSTA.sta.threshold.authmode = WIFI_AUTH_OPEN;
    }
    else
    {
        wifi_configSTA.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        strcpy((char*)wifi_configSTA.sta.password, FWCONFIG_MASTERBASE_PASS);
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_configSTA) );

    // Start AP + STA
    ESP_ERROR_CHECK(esp_wifi_start() );
}

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
                case EChevronAnimation::Suicide:
                {
                    ESP_LOGI(TAG, "Animation / Suicide");
                    // Light up the hidden chevron RED.
                    GPIO_SetPixel(SGURingNS::ChevronIndexToLedIndex(5), LED_OUTPUT_MAX, 0, 0);
                    GPIO_SetPixel(SGURingNS::ChevronIndexToLedIndex(6), LED_OUTPUT_MAX, 0, 0);
                    break;
                }
                case EChevronAnimation::FadeIn:
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
                case EChevronAnimation::FadeOut:
                {
                    ESP_LOGI(TAG, "Animation / FadeOut");
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
                case EChevronAnimation::ErrorToWhite:
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
                case EChevronAnimation::ErrorToOff:
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
                case EChevronAnimation::AllSymbolsOn:
                {
                    ESP_LOGI(TAG, "Animation / AllSymbolsOn");
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

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station %02x:%02x:%02x:%02x:%02x:%02x join, AID=%d",
            event->mac[0], event->mac[1],event->mac[2], event->mac[3],event->mac[4], event->mac[5],
            (int)event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station %02x:%02x:%02x:%02x:%02x:%02x leave, AID=%d",
            event->mac[0], event->mac[1],event->mac[2], event->mac[3],event->mac[4], event->mac[5],
            (int)event->aid);
    }
}

static void wifistation_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "retry to connect to the AP");
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
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
    ESP_LOGI(TAG, "BLE Update light received. Lights: %u", /*0*/(uint)psArg->u8LightCount);

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
        // ESP_LOGI(TAG, "Led index change: %d, Red: %d, Green: %d, Blue: %d", u8LightIndex, psArg->sRGB.u8Red, psArg->sRGB.u8Green, psArg->sRGB.u8Blue);
    }

    ResetAutoOffTicks();
}

static void SGUBRChevronsLightningHandler(const SChevronsLightningArg* psChevronLightningArg)
{
    ESP_LOGI(TAG, "BLE Chevron light received");

    // Not ready for chevron animation yet.
    m_s32ChevronAnim = (volatile int32_t)psChevronLightningArg->eChevronAnim;
    ResetAutoOffTicks();
}

static void SGUBRGotoFactory()
{
    ESP_LOGI(TAG, "Goto factory mode");

    const esp_partition_t* pFactoryPartition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);

    if (pFactoryPartition == NULL)
    {
        ESP_LOGE(TAG, "Factory partition cannot be found");
        return;
    }

    ESP_LOGI(TAG, "Set boot partition to factory mode");
    esp_ota_set_boot_partition(pFactoryPartition);
    esp_restart();

    ResetAutoOffTicks();
}

static void PingHandler(const SPingPongArg* psArg)
{
    ESP_LOGI(TAG, "Ping received: %" PRIu32, psArg->u32PingPong);
    if (m_UdpCommSocket < 0)
        return;

    uint8_t u8Payloads[128];
    const int length = SGUComm::EncPingPong(u8Payloads, sizeof(u8Payloads), psArg);
    int err = sendto(m_UdpCommSocket, u8Payloads, length, 0, (struct sockaddr *)&m_source_addr, sizeof(m_source_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Ping response sent");
}

static void MainTask(void *pvParameters)
{
    ESP_LOGI(TAG, "MainTask started ...");
    uint16_t u16Port = 7827;
    int sock = -1;
    {

    struct sockaddr_in dest_addr_ip4;
    dest_addr_ip4.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4.sin_family = AF_INET;
    dest_addr_ip4.sin_port = htons(u16Port);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        goto ERROR;
    }
    m_UdpCommSocket = sock;
    ESP_LOGI(TAG, "Socket created");

    int err = bind(sock, (struct sockaddr *)&dest_addr_ip4, sizeof(dest_addr_ip4));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        goto ERROR;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", (int)u16Port);
    /*struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10;
    setsockopt(pHandle->sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);*/
    uint8_t u8Buffers[64];
    SGUComm sSGUComm;

    while(true)
    {
        socklen_t socklen = sizeof(m_source_addr);
        int len = recvfrom(sock, u8Buffers, sizeof(u8Buffers), 0, (struct sockaddr *)&m_source_addr, &socklen);
        if (len > 0)
        {
            ESP_LOGI(TAG, "Receiving data, len: %d", (int)len);
            if (!SGUComm::Decode(m_sConfig, u8Buffers, len))
            {
                ESP_LOGE(TAG, "Unable to decode message");
            }
        }
        // 50 HZ
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    }
    ERROR:
    if (sock != -1) {
        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
    }
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

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    GPIO_Init();
    GPIO_EnableHoldPowerPin(true);

    InitESPNOW();

    // Clear LED strip (turn off all LEDs)
    //ESP_ERROR_CHECK(esp_pm_lock_acquire(lockHandle));
    //ESP_ERROR_CHECK(esp_pm_lock_release(lockHandle));

    xTaskCreatePinnedToCore(&LedRefreshTask, "RefreshLEDs", 4000, NULL, 10, NULL, 0);

    // Create task on CPU one ... to not interfere with FastLED
    // AppMain is created on task 0 by default.
    xTaskCreatePinnedToCore(&MainTask, "MainTask", 4000, NULL, 10, NULL, 1);

    long switchTicks = 0;
    bool bLastIsSuicide = false;

    while(true)
    {
        if (!m_bIsSuicide)
        {
            const bool bSwitchState = gpio_get_level((gpio_num_t)FWCONFIG_SWITCH_PIN);
            if (!bSwitchState) // Up
            {
                if (switchTicks == 0)
                    switchTicks = xTaskGetTickCount();

                // If we hold the switch long enough it stop the process.
                if ( (xTaskGetTickCount() - switchTicks) > pdMS_TO_TICKS(FWCONFIG_SWITCH_HOLDDELAY_MS))
                {
                    SGUBRGotoFactory();
                }
            }
            else
                switchTicks = 0; // Reset

            // Kill the power after 10 minutes maximum
            if ((xTaskGetTickCount() - m_lAutoOffTicks) > pdMS_TO_TICKS(m_ulAutoOffTimeoutMs))
            {
                m_bIsSuicide = true;
            }
        }

        // Means cutting to power to itself.
        if (m_bIsSuicide)
        {
            if (!bLastIsSuicide)
            {
                m_s32ChevronAnim = (volatile int32_t)EChevronAnimation::Suicide;
                ESP_LOGW(TAG, "Suicide animation");
            }

            // Release the power pin
            // Delay for animation before stop
            GPIO_EnableHoldPowerPin(false);
            // At this point if there are no external power to maintain it, it should die.
            vTaskDelay(pdMS_TO_TICKS(250));
            ESP_LOGW(TAG, "Seems like we will live");
        }

        bLastIsSuicide = m_bIsSuicide;
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
