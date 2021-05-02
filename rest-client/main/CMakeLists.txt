idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES fetch connect nvs_flash json
                    )