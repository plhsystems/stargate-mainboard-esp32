idf_component_register(SRCS "MainApp.cpp" "PabloSGHW.cpp"
    INCLUDE_DIRS "."
    REQUIRES main-app esp_driver_ledc esp_driver_gpio)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
