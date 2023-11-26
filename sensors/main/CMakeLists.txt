
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES troubleshoot_sensor temperature_sensor light_sensor led_control interrupt_task button load_switch
                    )