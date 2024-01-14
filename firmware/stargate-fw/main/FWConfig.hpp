#ifndef _FWCONFIG_H_
#define _FWCONFIG_H_

#define FWCONFIG_GATECONTROL_STACKSIZE 4500
#define FWCONFIG_GATECONTROL_PRIORITY_DEFAULT (tskIDLE_PRIORITY+1)
#define FWCONFIG_GATECONTROL_COREID 1

#define SETTINGS_EENTRY_WSTAIsActive (1)
#define SETTINGS_EENTRY_WSTASSID ""
#define SETTINGS_EENTRY_WSTAPass ""

// Soft access point
#define FWCONFIG_SOFTAP_WIFI_SSID_BASE ("SGU-%02X%02X%02X")
#define FWCONFIG_SOFTAP_WIFI_PASS      ""
#define FWCONFIG_SOFTAP_WIFI_CHANNEL   0
#define FWCONFIG_SOFTAP_MAX_CONN       10

#define FWCONFIG_RING_IPADDRESS "192.168.66.250"

#define FWCONFIG_SOFTAP_MAXIMUM_RETRY 10000

#endif