idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES connect server
                    )

# include the html folder in spiffs (partition, folder, flag/instruction)
spiffs_create_partition_image(web ../html FLASH_IN_PROJECT)