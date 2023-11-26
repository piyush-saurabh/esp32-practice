idf_component_register(
    SRCS "troubleshoot_sensor.c"
    SRCS "dht.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos log driver esp_adc_cal
    )