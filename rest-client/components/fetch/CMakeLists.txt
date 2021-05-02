idf_component_register(
    SRCS "fetch.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_client connect
    )