idf_component_register(SRCS "main.c" "gamepad.c" "CRSF.c"
    INCLUDE_DIRS "."
    REQUIRES tinyusb
    )
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
