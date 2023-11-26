#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <esp_log.h>

#include <esp_vfs.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>

// Pins Configuration
#define PIN_CS      5
#define PIN_MOSI    23
#define PIN_MISO    19
#define PIN_CLK     18

#define TAG "MAIN"

// Comment it to stop debugging
#define DEBUG

static const char *BASE_PATH  = "/store";

// Define function signature
void write_file(char *path, char *content);
void log_to_sdcard(char *path, char *content);
void read_file(char *path);

void app_main()
{
    // Configure how to handle the mounting
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true, // if mount fails, format it as FAT32
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Configure SPI bus
    spi_bus_config_t spi_bus_config = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_CLK,
        .quadhd_io_num = -1, // voltage level (depending on the SD card reader)
        .quadwp_io_num = -1 // write protection (depending on the SD card reader)

    };

    // Initialize the bus
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    ESP_ERROR_CHECK(spi_bus_initialize(host.slot, &spi_bus_config, SDSPI_DEFAULT_HOST));
    

    // Initialize the slot
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_CS;
    slot_config.host_id = host.slot;
    
    sdmmc_card_t *card;

    // Mount the card
    // ESP_ERROR_CHECK will show the error on serial console if there is no card
    ESP_ERROR_CHECK(esp_vfs_fat_sdspi_mount(BASE_PATH, &host, &slot_config, &mount_config, &card));
    
    // Print info about SD Card
    sdmmc_card_print_info(stdout, card);

    // write_file("/store/text.csv", "timestamp,voltage");
    // write_file("/store/text.csv", "12:00,3.5");
    // write_file("/store/text.csv", "12:15,3.6");
    // write_file("/store/text.csv", "12:666,4.2");

    log_to_sdcard("/store/logs-1.csv", "timestamp,voltage\n");
    log_to_sdcard("/store/logs-1.csv", "13:00,3.5\n");
    log_to_sdcard("/store/logs-1.csv", "14:15,3.6\n");
    log_to_sdcard("/store/logs-1.csv", "15:66,4.2\n");

    
    read_file("/store/logs-1.csv");

    // Unmount the card
    esp_vfs_fat_sdcard_unmount(BASE_PATH, card);
    ESP_LOGI(TAG, "Card unmounted");


    // Relese the SPI bus
    spi_bus_free(host.slot);

}

// Read the file
void read_file(char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *file = fopen(path, "r");
    char buffer[100];
    fgets(buffer, 99, file);
    fclose(file);
    ESP_LOGI(TAG, "File contains: %s", buffer);

}

// Write to the file
void write_file(char *path, char *content)
{
    ESP_LOGI(TAG, "Writing \"%s\" to \"%s\"", content, path);
    FILE *file = fopen(path, "w");
    fputs(content, file);
    fclose(file);
}

// Log the data to a file in SD Card
void log_to_sdcard(char *path, char *content)
{
    ESP_LOGI(TAG, "Append \"%s\" to \"%s\"", content, path);
    FILE *file = fopen(path, "a+");
    fputs(content, file);
    fclose(file);
}