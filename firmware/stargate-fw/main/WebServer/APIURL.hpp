#pragma once

// MISC API
#define APIURL_GETSTATUS_URI        "/api/getstatus"
#define APIURL_GETSYSINFO_URI       "/api/getsysinfo"

// Sound API
#define APIURL_GETSOUNDLIST_URI     "/api/sound/list"
#define APIURL_PLAYSOUND_URI        "/api/sound/play"
#define APIURL_STOPSOUND_URI        "/api/sound/stop"

// Controls API
#define APIURL_POSTCONTROL_AUTOHOME_URI         "/api/control/autohome"
#define APIURL_POSTCONTROL_AUTOCALIBRATE_URI    "/api/control/autocalibrate"
#define APIURL_POSTCONTROL_KEYPRESS_URI         "/api/control/keypress"
#define APIURL_POSTCONTROL_DIALADDRESS_URI      "/api/control/dialaddress"
#define APIURL_POSTCONTROL_MANUALWORMHOLE_URI   "/api/control/manualwormhole"
#define APIURL_POSTCONTROL_ABORT_URI            "/api/control/abort"

// Test the hardware
#define APIURL_POSTCONTROL_TESTRAMPLIGHT_URI    "/api/control/testramplight"
#define APIURL_POSTCONTROL_TESTSERVO_URI        "/api/control/testservo"

// Ring control
#define APIURL_POSTRINGCONTROL_POWEROFF_URI     "/api/ringcontrol/poweroff"
#define APIURL_POSTRINGCONTROL_TESTANIMATE_URI  "/api/ringcontrol/testanimate"
#define APIURL_POSTRINGCONTROL_GOTOFACTORY_URI  "/api/ringcontrol/gotofactory"

#define APIURL_GETPOST_SETTINGSJSON_URI         "/api/settingsjson"
#define APIURL_GETFREERTOSDBGINFO_URI           "/api/getfreertosdbginfo"

#define APIURL_GALAXY_GETINFO_MILKYWAY_URI       "/api/galaxy/getinfo/milkyway"
#define APIURL_GALAXY_GETINFO_PEGASUS_URI        "/api/galaxy/getinfo/pegasus"
#define APIURL_GALAXY_GETINFO_UNIVERSE_URI       "/api/galaxy/getinfo/universe"

#define APIURL_POST_OTAUPLOAD_URI               "/ota/upload"