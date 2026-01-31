idf_component_register( SRCS
    "MainApp.cpp" "PinkySGHW.cpp"
    INCLUDE_DIRS "."
    REQUIRES main-app esp_driver_mcpwm esp_driver_ledc esp_driver_gpio esp_driver_gptimer)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers -fexceptions)
