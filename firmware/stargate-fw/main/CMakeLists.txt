idf_component_register( SRCS
    "MainApp.cpp" "WiFiMgr.cpp" "Settings.cpp" "HttpClient.cpp"
    "GateControl/GateControl.cpp"
    "Audio/SoundFX.cpp"
    "Ring/RingComm.cpp"
    "DHD/USB_DHD.cpp"
    "WebServer/WebServer.cpp" "WebServer/SystemAPI.cpp" "WebServer/OTA_API.cpp" "WebServer/GateAPI.cpp" "WebServer/EmbeddedFiles.c"
    "Gate/BaseGate.cpp" "Gate/MilkyWayGate.cpp" "Gate/PegasusGate.cpp" "Gate/UniverseGate.cpp"
    "Gate/GateFactory.cpp"
    "Wormhole/Wormhole.cpp"
    "HW/PinkySGHW.cpp"
    INCLUDE_DIRS "."
    REQUIRES sgu-ringcomm nvsjson misc-formula esp_http_client esp-tls driver led_strip esp_rom lwip nvs_flash json esp_http_server esp_wifi app_update fatfs esp_rom esp_timer esp_hw_support esp_common log)
