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
xSemaphoreHandle connectionSemaphore;

// Binary semaphore for wifi initialization
// Add this to connect.h
xSemaphoreHandle initSemaphore;

// Tag for logging
char *TAG = "MAIN";


// Task handler
void onConnected(void *param)
{
    while (true)
    {
        // If wifi is connected. Try getting the semaphore for 10sec
        // similar to wait. It will wait till xSemaphoreGive is invoked from some other task
        if (xSemaphoreTake(connectionSemaphore, portMAX_DELAY))
        {
            // do something useful
            // Start the HTTP server
            RegisterEndPoints();

            //xSemaphoreTake(connectionSemaphore, portMAX_DELAY);
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
    connectionSemaphore = xSemaphoreCreateBinary();

    // Initialize the wifi. 
    // Without calling the function directly, we can use FreeRTOS task so that we can control it using semaphore
    // wifiInit();

    // Task for initializing wifi
    initSemaphore = xSemaphoreCreateBinary();
    xTaskCreate(&wifiInit, "init comms", 1024 * 3, NULL, 10, NULL);
    xSemaphoreGive(initSemaphore);

    // create a task for making HTTP request
    xTaskCreate(&onConnected, "On Connected", 1024 * 5, NULL, 5, NULL);
}