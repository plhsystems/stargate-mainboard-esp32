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
    REQUIRES sgu-ringcomm nvsjson misc-formula esp_http_client esp-tls driver esp_driver_uart led_strip esp_rom nvs_flash cjson esp_http_server esp_wifi app_update fatfs esp_rom esp_timer esp_hw_support esp_common log bt)

# Run embeddedgen.py to embed www resources before compiling
idf_build_get_property(python PYTHON)
set(EMBEDDEDGEN_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/../../tools/embeddedgen.py")
set(EMBEDDEDGEN_INPUT  "${CMAKE_CURRENT_LIST_DIR}/webserver-assets")
set(EMBEDDEDGEN_OUTPUT "${CMAKE_CURRENT_LIST_DIR}/WebServer/EmbeddedFiles.c")

file(GLOB_RECURSE WEBSERVER_ASSETS "${EMBEDDEDGEN_INPUT}/*")

add_custom_command(
    OUTPUT  "${EMBEDDEDGEN_OUTPUT}"
    COMMAND ${python} "${EMBEDDEDGEN_SCRIPT}" -i "${EMBEDDEDGEN_INPUT}" -o "${CMAKE_CURRENT_LIST_DIR}/WebServer" --compress-all
    DEPENDS ${WEBSERVER_ASSETS}
    COMMENT "Generating embedded web files..."
)

add_custom_target(embeddedgen DEPENDS "${EMBEDDEDGEN_OUTPUT}")
add_dependencies(${COMPONENT_LIB} embeddedgen)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers -fexceptions)
