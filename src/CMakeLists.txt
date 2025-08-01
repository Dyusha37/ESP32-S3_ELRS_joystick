idf_component_register(SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES tinyusb
    )
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
