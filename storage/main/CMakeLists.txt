set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

set(COMPONENT_EMBED_TXTFILES
    "../files/index.html"
    "../files/sample.txt"
)

set(COMPONENT_EMBED_FILES
    "../files/pinout.png"
)

register_component()

# This should be the last line else we will get error
spiffs_create_partition_image(my_spiffs_storage ../spiffs_dir FLASH_IN_PROJECT)