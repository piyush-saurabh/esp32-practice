#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h> // for connecting/disconnecting from wifi
#include <esp_log.h>

// Connect to wifi
#include "connect.h"

// Custom HTTP Server
#include "server.h"

// Binary semaphore for the task when wifi is connected
xSemaphoreHandle onConnectionHandler;

// Tag for logging
char *TAG = "MAIN";


// Task handler: Make HTTP request
void onConnected(void *param)
{
    while (true)
    {
        // If wifi is connected. Try getting the semaphore for 10sec
        // similar to wait. It will wait till xSemaphoreGive is invoked from some other task
        if (xSemaphoreTake(onConnectionHandler, 10 * 1000 / portTICK_PERIOD_MS))
        {
            // do something useful
            // Start the HTTP server
            RegisterEndPoints();
            

            // Don't disconnect from wifi because we want to keep listening
            // esp_wifi_disconnect();

            xSemaphoreTake(onConnectionHandler, portMAX_DELAY);
        }
        else
        {
            // Wifi connection failed.
            // Restart the chip
            ESP_LOGE(TAG, "Failed to connect to the wifi. Retry in");
            for (int i = 0; i < 5; i++)
            {
                ESP_LOGE(TAG, "...%d", i);
                vTaskDelay(1000 / portTICK_RATE_MS);
            }
            esp_restart();
        }
    }
}

void app_main(void)
{
    // create binary semaphore for handling task when wifi is connected
    onConnectionHandler = xSemaphoreCreateBinary();

    // Initialize the wifi
    wifiInit();

    // create a task for making HTTP request
    xTaskCreate(&onConnected, "On Connected", 1024 * 5, NULL, 5, NULL);
}