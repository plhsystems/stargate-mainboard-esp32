idf_component_register(SRCS "MainApp.cpp" "PabloSGHW.cpp"
    INCLUDE_DIRS "."
    REQUIRES main-app)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
