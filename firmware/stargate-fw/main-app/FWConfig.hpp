#pragma once

#define FWCONFIG_MAINTASK_PRIORITY_DEFAULT (tskIDLE_PRIORITY+1)
#define FWCONFIG_WEBSERVERTASK_PRIORITY_DEFAULT (tskIDLE_PRIORITY+2)

#define FWCONFIG_RINGCOMM_STACKSIZE 4500
#define FWCONFIG_RINGCOMM_PRIORITY_DEFAULT (tskIDLE_PRIORITY)
#define FWCONFIG_RINGCOMM_COREID 1

#define FWCONFIG_GATECONTROL_STACKSIZE 4500
#define FWCONFIG_GATECONTROL_PRIORITY_DEFAULT (tskIDLE_PRIORITY+1)
#define FWCONFIG_GATECONTROL_COREID 1

#define FWCONFIG_HTTPCLIENT_STACKSIZE 4500
#define FWCONFIG_HTTPCLIENT_PRIORITY_DEFAULT (tskIDLE_PRIORITY+1)
#define FWCONFIG_HTTPCLIENT_COREID 1
#define FWCONFIG_HTTPCLIENT_FASTATTEMPT_COUNT (10)

// Soft access point
//#define FWCONFIG_SOFTAP_WIFI_SSID_BASE ("SGU-%02X%02X%02X")
#define FWCONFIG_SOFTAP_WIFI_SSID ("Stargate-Universe")
#define FWCONFIG_SOFTAP_WIFI_PASS      ""
#define FWCONFIG_SOFTAP_WIFI_CHANNEL   0
#define FWCONFIG_SOFTAP_MAX_CONN       10

#define FWCONFIG_RING_IPADDRESS "192.168.66.250"
#define FWCONFIG_RING_PORT 7827


#define FWCONFIG_SOFTAP_MAXIMUM_RETRY 10000