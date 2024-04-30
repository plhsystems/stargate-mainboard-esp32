idf_component_register(SRCS "gpio.c" "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES sgu-ringcomm misc-formula lwip log esp_common esp_app_format esp_event esp_wifi esp_hw_support nvs_flash esp_pm driver app_update led_strip)
