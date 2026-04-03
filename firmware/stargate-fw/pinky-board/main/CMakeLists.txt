idf_component_register( SRCS
    "MainApp.cpp" "PinkySGHW.cpp"
    INCLUDE_DIRS "."
    REQUIRES main-app driver esp_driver_mcpwm esp_driver_ledc esp_driver_uart esp_driver_rmt esp_driver_gpio)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers -fexceptions)
