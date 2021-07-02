
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES temperature_sensor led_control interrupt_task button
                    )