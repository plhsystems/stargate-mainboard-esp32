idf_component_register(
    SRCS "MainApp.cpp" "GateControl.cpp" "WiFiMgr.cpp" "Settings.cpp" "HttpClient.cpp"
    "Audio/SoundFX.cpp"
    "Ring/RingComm.cpp"
    "WebServer/WebServer.cpp" "WebServer/SystemAPI.cpp" "WebServer/OTA_API.cpp" "WebServer/GateAPI.cpp" "WebServer/EmbeddedFiles.c"
    "Gate/BaseGate.cpp" "Gate/MilkyWayGate.cpp" "Gate/PegasusGate.cpp" "Gate/UniverseGate.cpp"
    "Gate/GateFactory.cpp"
    "Wormhole/Wormhole.cpp"
    "HW/SGHW_HAL.cpp" "HW/PinkySGHW.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvsjson misc-formula esp_http_client esp-tls driver led_strip esp_rom lwip nvs_flash json esp_http_server esp_wifi app_update fatfs esp_rom esp_timer esp_hw_support esp_common log)
