idf_component_register( SRCS
    "App.cpp" "WifiMgr.cpp" "Settings.cpp" "HttpClient.cpp"
    "Audio/SoundFX.cpp"
    "Ring/RingBLEClient.cpp"
    "WebServer/WebServer.cpp" "WebServer/SystemAPI.cpp" "WebServer/OTA_API.cpp" "WebServer/GateAPI.cpp" "WebServer/EmbeddedFiles.c"
    "Gate/GateControl.cpp" "Gate/Chevron.hpp"
    "Gate/BaseGate.cpp" "Gate/MilkyWayGate.cpp" "Gate/PegasusGate.cpp" "Gate/UniverseGate.cpp"
    "Gate/GateFactory.cpp"
    "Wormhole/Wormhole.cpp"
    INCLUDE_DIRS "."
    REQUIRES sgu-ringcomm nvsjson misc-formula esp_http_client esp-tls driver led_strip esp_rom nvs_flash cjson esp_http_server esp_wifi app_update fatfs esp_rom esp_timer esp_hw_support esp_common log bt esp_driver_uart)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers -fexceptions)
