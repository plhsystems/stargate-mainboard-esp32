#pragma once

/**
 * @brief Task Priority Configuration
 *
 * FreeRTOS priorities (higher number = higher priority).
 * configMAX_PRIORITIES is typically 25 in ESP-IDF.
 *
 * Priority hierarchy rationale:
 * - GateControl (5): Highest - stepper timing is critical for smooth motion
 * - RingBLEClient (4): High - BLE connection maintenance needs responsiveness
 * - SoundFX (3): Medium-High - audio sync with gate events
 * - HttpClient (2): Medium - external API calls can tolerate latency
 * - WebServer (1): Low - user interface can tolerate some delay
 * - MainTask (0): Idle - just toggles sanity LED
 *
 * All real-time motor control runs on Core 1 to avoid WiFi/BLE stack interference.
 */

#define FWCONFIG_MAINTASK_PRIORITY_DEFAULT      (tskIDLE_PRIORITY)
#define FWCONFIG_WEBSERVERTASK_PRIORITY_DEFAULT (tskIDLE_PRIORITY + 1)

#define FWCONFIG_RINGCOMM_STACKSIZE 4500
#define FWCONFIG_RINGCOMM_PRIORITY_DEFAULT      (tskIDLE_PRIORITY + 4)
#define FWCONFIG_RINGCOMM_COREID 1

#define FWCONFIG_GATECONTROL_STACKSIZE 8192
#define FWCONFIG_GATECONTROL_PRIORITY_DEFAULT   (tskIDLE_PRIORITY + 5)
#define FWCONFIG_GATECONTROL_COREID 1

#define FWCONFIG_HTTPCLIENT_STACKSIZE 4500
#define FWCONFIG_HTTPCLIENT_PRIORITY_DEFAULT    (tskIDLE_PRIORITY + 2)
#define FWCONFIG_HTTPCLIENT_COREID 1
#define FWCONFIG_HTTPCLIENT_FASTATTEMPT_COUNT (10)

#define FWCONFIG_SOUNDFX_PRIORITY_DEFAULT       (tskIDLE_PRIORITY + 3)

// Soft access point
//#define FWCONFIG_SOFTAP_WIFI_SSID_BASE ("SGU-%02X%02X%02X")
#define FWCONFIG_SOFTAP_WIFI_SSID ("Stargate-Universe")
#define FWCONFIG_SOFTAP_WIFI_PASS      ""
#define FWCONFIG_SOFTAP_WIFI_CHANNEL   0
#define FWCONFIG_SOFTAP_MAX_CONN       10

#define FWCONFIG_RING_IPADDRESS "192.168.66.250"
#define FWCONFIG_RING_PORT 7827


#define FWCONFIG_SOFTAP_MAXIMUM_RETRY 10000
