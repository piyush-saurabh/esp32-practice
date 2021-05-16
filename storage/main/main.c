#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spiffs.h" // For accessing spiffs

// For listing the files
#include <stdio.h>
#include <dirent.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#define TAG "NVS"

// Storing files (txt, image) on the chip
void add_files_to_flash()
{
    // Get the handler for index.html
    // replace any special character (.) with _
    // Text file are null terminated so we can print them
    extern const unsigned char indexHtml[] asm("_binary_index_html_start");
    printf("html = %s\n", indexHtml);

    // Text file are null terminated so we can print them
    extern const unsigned char sample[] asm("_binary_sample_txt_start");
    printf("sample = %s\n", sample);

    // Binary file are not null terminated, so we cannot print them.
    // Find the size of the file
    extern const unsigned char imgStart[] asm("_binary_pinout_png_start");
    extern const unsigned char imgEnd[] asm("_binary_pinout_png_end");
    const unsigned int imgSize = imgEnd - imgStart;
    printf("img size is %d\n", imgSize);
}

// Simple read and write to NVS
void nvs_demo()
{
    // initialize the nvs flash
    ESP_ERROR_CHECK(nvs_flash_init());

    // open the nvs
    // 1st param is namespace. we can have multiple key-value pair in different namespace without collision
    // 2nd param: we want to read and write to this nvs
    // 3rd param: handle to perform operation on nvs
    nvs_handle handle;
    ESP_ERROR_CHECK(nvs_open("store", NVS_READWRITE, &handle));

    // Read from nvs (simple integer value)
    // Specify the key stored in nvs here 'mykey'
    int32_t myval = 0;
    esp_err_t result = nvs_get_i32(handle, "mykey", &myval);

    switch (result)
    {
    case ESP_ERR_NVS_NOT_FOUND:
    case ESP_ERR_NOT_FOUND:
        ESP_LOGE(TAG, "Value not set yet");
        break;
    case ESP_OK:
        ESP_LOGI(TAG, "Value is %d", myval);
        break;
    default:
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(result));
        break;
    }

    // Calculate the number of chip reset
    myval++;

    // Write to NVS
    ESP_ERROR_CHECK(nvs_set_i32(handle, "mykey", myval));

    // Commit the changes to nvs
    ESP_ERROR_CHECK(nvs_commit(handle));

    // Close the nvs handle
    nvs_close(handle);
}

// Custom NVS partition
void custom_nvs_demo()
{
    // initialize the particular nvs flash (check the name from partition.csv file)
    ESP_ERROR_CHECK(nvs_flash_init_partition("my_custom_nvs"));

    // Open the selected partition
    nvs_handle handle;
    ESP_ERROR_CHECK(nvs_open_from_partition("my_custom_nvs", "store", NVS_READWRITE, &handle));

    // Get stats of nvs partition
    nvs_stats_t nvsStats;
    nvs_get_stats("my_custom_nvs", &nvsStats); // for default partition, use NULL
    ESP_LOGI(TAG, "Used: %d bytes, Free: %d bytes, Total: %d bytes, Namespace count: %d", nvsStats.used_entries,
             nvsStats.free_entries, nvsStats.total_entries, nvsStats.namespace_count);

    // Rest of the code will be same
    // Read from nvs (simple integer value)
    // Specify the key stored in nvs here 'mykey'
    int32_t myval = 0;
    esp_err_t result = nvs_get_i32(handle, "mykey", &myval);

    switch (result)
    {
    case ESP_ERR_NVS_NOT_FOUND:
    case ESP_ERR_NOT_FOUND:
        ESP_LOGE(TAG, "Value not set yet");
        break;
    case ESP_OK:
        ESP_LOGI(TAG, "Value is %d", myval);
        break;
    default:
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(result));
        break;
    }

    // Calculate the number of chip reset
    myval++;

    // Write to NVS
    ESP_ERROR_CHECK(nvs_set_i32(handle, "mykey", myval));

    // Commit the changes to nvs
    ESP_ERROR_CHECK(nvs_commit(handle));

    // Close the nvs handle
    nvs_close(handle);
}

// Structure we want to store in nvs
typedef struct cat_struct
{
    char name[30];
    int age;
    int id;
} Cat;

// Store structure in nvs partition
void nvs_struct_demo()
{
    // initialize the particular nvs flash (check the name from partition.csv file)
    ESP_ERROR_CHECK(nvs_flash_init_partition("my_custom_nvs"));

    // Open the selected partition
    nvs_handle handle;
    ESP_ERROR_CHECK(nvs_open_from_partition("my_custom_nvs", "cat_store", NVS_READWRITE, &handle));

    // Required for reading the struct from nvs
    char catKey[50];              // max key size can be 15 bytes as per IDF doc
    Cat cat;                      // create a cat object
    size_t catSize = sizeof(Cat); // Size of the structure

    // Read 5 objects from the nvs
    for (int i = 0; i < 5; i++)
    {
        // Set the unique key for nvs
        sprintf(catKey, "cat_%d", i);

        // Use blob to get the struct
        // Specify the key. It should be 15 bytes unique string
        esp_err_t result = nvs_get_blob(handle, catKey, (void *)&cat, &catSize);

        // Read the result
        switch (result)
        {
        case ESP_ERR_NOT_FOUND:
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "Value not set yet");
            break;
        case ESP_OK:
            ESP_LOGI(TAG, "cat name: %s, age %d, id %d", cat.name, cat.age, cat.id);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(result));
            break;
        }
    }

    // Write 5 structs in nvs
    for (int i = 0; i < 5; i++)
    {
        sprintf(catKey, "cat_%d", i);

        Cat newCat;
        sprintf(newCat.name, "Mr cat %d", i);

        newCat.age = i + 2;
        newCat.id = i;

        // Write to NVS
        ESP_ERROR_CHECK(nvs_set_blob(handle, catKey, (void *)&newCat, sizeof(Cat)));

        // Commit the changes to nvs
        ESP_ERROR_CHECK(nvs_commit(handle));
    }

    // Close the nvs handle
    nvs_close(handle);
}

// Spiffs usage
void spiff_demo()
{
    // Configuration for using spiffs
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",  // root directory how it will be flashed on the chip
        .partition_label = NULL, // use the spiffs label specified in partitions.csv
        .max_files = 5,          // no of files which can be opened at a time
        .format_if_mount_failed = true,
    };

    // Register the spiffs with above configuration
    esp_vfs_spiffs_register(&config);

    // Find the list of all the files in a directory
    // Open the directory
    DIR *dir = opendir("/spiffs");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        char fullPath[300];
        sprintf(fullPath, "/spiffs/%s", entry->d_name);

        // Get the statics of all the files
        struct stat entryStat;
        if (stat(fullPath, &entryStat) == -1)
        {
            ESP_LOGE("SPIFFS", "Error getting statistics for %s", fullPath);
        }
        else
        {
            ESP_LOGI("SPIFFS", "Full Path = %s, File Size = %ld bytes", fullPath, entryStat.st_size);
        }
    }

    // SPIFFS usage statistics
    size_t total = 0, used = 0;
    esp_spiffs_info(NULL, &total, &used);
    ESP_LOGI("SPIFFS", "Total Size = %d bytes, Used = %d bytes", total, used);

    // Read file
    FILE *file = fopen("/spiffs/testdir/testfile.txt", "r");

    if (file == NULL)
    {
        ESP_LOGE("SPIFFS", "File cannot be opened");
    }
    else
    {
        char line[256];

        // Start reading the file line by line
        while (fgets(line, sizeof(line), file) != NULL)
        {
            printf(line);
        }

        // close the file
        fclose(file);
    }

    // unregister the spiffs to clean it up (first partition)
    esp_vfs_spiffs_unregister(NULL);
}



void app_main(void)
{
    //add_files_to_flash();

    // Read and write to nvs storage
    //nvs_demo();

    // custom nvs
    //custom_nvs_demo();

    // store structure in nvs
    //nvs_struct_demo();

    // Read file from spiffs
    spiff_demo();
}