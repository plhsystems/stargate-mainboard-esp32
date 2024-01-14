idf_component_register(SRCS "MainApp.cpp" "GateControl.cpp" "WiFiMgr.cpp"
    "WebServer/WebServer.cpp"
    "Gate/BaseGate.cpp" "Gate/KristianGate.cpp" "Gate/AtlantisGate.cpp" "Gate/UniverseGate.cpp"
    "Gate/GateFactory.cpp"
    "Wormhole/Wormhole.cpp"
    "HW/BoardHW.cpp"
    INCLUDE_DIRS "."
    REQUIRES driver led_strip esp_rom lwip nvs_flash json esp_http_server esp_wifi app_update fatfs esp_rom esp_timer esp_hw_support esp_common log)
