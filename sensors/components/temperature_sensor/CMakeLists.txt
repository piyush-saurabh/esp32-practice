idf_component_register(
    SRCS "temperature_sensor.c"
    SRCS "dht.c"
    INCLUDE_DIRS "include"
    REQUIRES driver freertos log sd_card
    )