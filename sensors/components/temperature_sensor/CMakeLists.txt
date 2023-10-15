idf_component_register(
    SRCS "temperature_sensor.c"
    SRCS "dht.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_client driver freertos log
    )