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

#define TAG "OTA"

// Semaphore for controlling the ota task
xSemaphoreHandle ota_semaphore;

// Software version
const int software_version = 2;

// Flash memory location for the stored server certificate (cert pinning)
// naming convention _binary_file_extension_start
extern const uint8_t server_cert_pem_start[] asm("_binary_github_cer_start");

esp_err_t  client_event_handler(esp_http_client_event_t *evt)
{
  return ESP_OK;
}

// FreeRTOS task for OTA
void run_ota(void *params)
{
    while (true)
    {
        // This task will wait here till we get the semaphore
        // We get the semaphore from ISR (button press)
        xSemaphoreTake(ota_semaphore, portMAX_DELAY);
        ESP_LOGI(TAG, "Invoking OTA");

        // Connect to the internet
        ESP_ERROR_CHECK(nvs_flash_init());
        tcpip_adapter_init();
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        ESP_ERROR_CHECK(example_connect());

        // Use HTTP Client to fetch the OTA
        esp_http_client_config_t clientConfig = {
            .url = "https://github.com/piyush-saurabh/esp32-practice/raw/main/ota/release/v1/ota.bin", // our ota location
            .event_handler = client_event_handler,
            .cert_pem = (char *)server_cert_pem_start // verify the server certificate
        };

        // Get the binary for OTA
        if (esp_https_ota(&clientConfig) == ESP_OK)
        {
            // The new binary is written to the OTA partition
            ESP_LOGI(TAG, "OTA flash succsessfull for version %d.", software_version);
            printf("restarting in 5 seconds\n");
            vTaskDelay(pdMS_TO_TICKS(5000));

            // Restart the chip
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
    printf("Congrats!! You are latest version of firmware\n");

    // Print the current software version on starting the application
    ESP_LOGI("SOFTWARE VERSION", "we are running %d", software_version);

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