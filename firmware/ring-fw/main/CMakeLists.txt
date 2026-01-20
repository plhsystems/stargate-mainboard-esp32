idf_component_register(SRCS "gpio.c" "main.cpp" "ble_server.c"
    INCLUDE_DIRS "."
    REQUIRES sgu-ringcomm misc-formula log esp_common esp_app_format esp_event esp_hw_support nvs_flash esp_pm driver app_update led_strip bt)
