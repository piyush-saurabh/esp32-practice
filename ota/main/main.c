#include <stdio.h>
#include <string.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_http_client.h>
#include <esp_https_ota.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <driver/gpio.h>              // use trigger to start the update
#include "protocol_examples_common.h" // quickly connect to internet using this example
#include <esp_ota_ops.h>

#define TAG "OTA"

// Semaphore for controlling the ota task
xSemaphoreHandle ota_semaphore;

// Software version
//const int software_version = 1;

// Flash memory location for the stored server certificate (cert pinning)
// naming convention _binary_file_extension_start
extern const uint8_t server_cert_pem_start[] asm("_binary_github_cer_start");

esp_err_t  client_event_handler(esp_http_client_event_t *evt)
{
  return ESP_OK;
}

// Function to validate the if the new OTA is different from the current one
esp_err_t validate_image_header(esp_app_desc_t *incoming_ota_desc)
{
    // Get the current version of OTA
    // TODO refactor this to create a separate function for getting the current version
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    esp_app_desc_t running_partition_description;
    esp_ota_get_partition_description(running_partition, &running_partition_description);

    ESP_LOGI(TAG, "current version is %s\n", running_partition_description.version);
    ESP_LOGI(TAG, "new version is %s\n", incoming_ota_desc->version);

    // Check if the current version and the new version are same
    if (strcmp(running_partition_description.version, incoming_ota_desc->version) == 0)
    {
        ESP_LOGW(TAG, "NEW VERSION IS THE SAME AS CURRENT VERSION. ABORTING");
        return ESP_FAIL;
    }
    return ESP_OK;
}

// FreeRTOS task for OTA
void run_ota(void *params)
{
    // Connect to the internet
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    while (true)
    {
        // This task will wait here till we get the semaphore
        // We get the semaphore from ISR (button press)
        xSemaphoreTake(ota_semaphore, portMAX_DELAY);
        ESP_LOGI(TAG, "Invoking OTA");

        
        ESP_ERROR_CHECK(example_connect());

        // Use HTTP Client to fetch the OTA
        esp_http_client_config_t clientConfig = {
            .url = "https://github.com/piyush-saurabh/esp32-practice/raw/main/ota/release/v2/ota.bin", // our ota location
            .event_handler = client_event_handler,
            .cert_pem = (char *)server_cert_pem_start // verify the server certificate
        };

        /* Old Code: Automated OTA
        // Get the binary for OTA
        if (esp_https_ota(&clientConfig) == ESP_OK)
        {
            // The new binary is written to the OTA partition
            //ESP_LOGI(TAG, "OTA flash succsessfull for version %d.", software_version);
            printf("restarting in 5 seconds\n");
            vTaskDelay(pdMS_TO_TICKS(5000));

            // Restart the chip
            esp_restart();
        }
        */

        // Do the OTA manually
        // Get OTA configuration structure
        esp_https_ota_config_t ota_config = {
        .http_config = &clientConfig};

        // Create OTA handle
        esp_https_ota_handle_t ota_handle = NULL;

        // Begin the OTA
        if (esp_https_ota_begin(&ota_config, &ota_handle) != ESP_OK)
        {
            ESP_LOGE(TAG, "esp_https_ota_begin failed");
            example_disconnect();
            continue;
        }

        // Get more info of the OTA being downloaded
        esp_app_desc_t incoming_ota_desc;
        if (esp_https_ota_get_img_desc(ota_handle, &incoming_ota_desc) != ESP_OK)
        {
            ESP_LOGE(TAG, "esp_https_ota_get_img_desc failed");
            esp_https_ota_finish(ota_handle);
            example_disconnect();
            continue;
        }

        // Validate that the incoming OTA is not same as the current OTA
        // validate_image_header() is the function with the custom logic for the validation
        if (validate_image_header(&incoming_ota_desc) != ESP_OK)
        {
            // If the OTA are same, don't download the OTA
            ESP_LOGE(TAG, "validate_image_header failed");
            esp_https_ota_finish(ota_handle);

            // disconnect from the internet
            example_disconnect();

            // Go to the start of of FreeRTOS task while(true) loop because both the version of OTA are same. So do not download the OTA
            continue;
        }

        // Since the incoming OTA are different, download the OTA
        while (true)
        {
            // Download the OTA
            // Download will happen in chunks. Below method will be called multiple times (so while loop)
            esp_err_t ota_result = esp_https_ota_perform(ota_handle);
            if (ota_result != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
                break;
        }

        // Finish the OTA
        if (esp_https_ota_finish(ota_handle) != ESP_OK)
        {
            ESP_LOGE(TAG, "esp_https_ota_finish failed");
            example_disconnect();
            continue;
        }
        else
        {
            // After OTA is finished, restart
            printf("restarting in 5 seconds\n");
            vTaskDelay(pdMS_TO_TICKS(5000));
            esp_restart();
        }

        // Error
        ESP_LOGE(TAG, "Failed to update firmware");
    }
}

// Handler when button is pushed
void on_button_pushed(void *params)
{
    // A semaphore is released from an interrupt
    xSemaphoreGiveFromISR(ota_semaphore, pdFALSE);
}

void app_main(void)
{
    //printf("You are running old version. Please update\n");
    printf("Congrats!! This is the latest version of the firmware\n");

    // Print the current software version on starting the application
    //ESP_LOGI("SOFTWARE VERSION", "we are running %d", software_version);

    // Get the software version from ESP
    // Get the currently running partition
    // TODO refactor this to create a separate function for getting the current version
    const esp_partition_t *running_partition = esp_ota_get_running_partition();

    // Get the description of currently running partition
    esp_app_desc_t running_partition_description;
    esp_ota_get_partition_description(running_partition, &running_partition_description);

    printf("current firmware version is: %s\n", running_partition_description.version);

    // Use button on ESP32 to trigger the interrupt
    // GPIO 0 is BOOT button on ESP32 DOIT DevKit v1 (30 pins)
    gpio_config_t gpioConfig = {
        .pin_bit_mask = 1ULL << GPIO_NUM_0,
        .mode = GPIO_MODE_DEF_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE};

    gpio_config(&gpioConfig);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_0, on_button_pushed, NULL);

    ota_semaphore = xSemaphoreCreateBinary();

    // FreeRTOS task for OTA update
    // This task is triggered by ISR
    xTaskCreate(run_ota, "run_ota", 1024 * 8, NULL, 2, NULL);
}