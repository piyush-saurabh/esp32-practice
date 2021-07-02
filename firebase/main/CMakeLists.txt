
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES http_connect wifi_connect http_stream json
                    EMBED_TXTFILES "../cert/firebase/auth.pem"
                    EMBED_TXTFILES "../cert/firebase/realtimedb.pem"
                    )