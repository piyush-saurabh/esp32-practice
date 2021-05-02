#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h>  // for connecting to the internet
#include <nvs_flash.h> // used by wifi internally to store information
#include <esp_http_client.h>
#include <esp_log.h>
#include <esp_event_loop.h>

#include <cJSON.h> // JSON parsing library

//#include <esp_event.h> // wifi uses event loop to act on the events associated with wifi (depricated)
#include <esp_netif.h>

// Rest client
#include "fetch.h"

// Connect to wifi
#include "connect.h"


// Binary semaphore for the task when wifi is connected
xSemaphoreHandle onConnectionHandler;

// Tag for logging
char *TAG = "MAIN";

// Parse the JSON response
void parseResponse(char *incomingBuffer, char * output)
{
    cJSON *payload = cJSON_Parse(incomingBuffer);
    cJSON *contents = cJSON_GetObjectItem(payload, "contents");
    cJSON *quotes = cJSON_GetObjectItem(contents, "quotes");

    // Iterate over quotes array
    cJSON *quotesElement;
    cJSON_ArrayForEach(quotesElement, quotes)
    {
        cJSON *quote = cJSON_GetObjectItem(quotesElement, "quote");
        ESP_LOGI(TAG,"%s",quote->valuestring);

        // do something with quote
        strcpy(output, quote->valuestring);
    }

    // clean cJSON
    cJSON_Delete(payload);
}


// Task handler: Make HTTP request
void onConnected(void *param)
{
    while(true)
    {
        // If wifi is connected. Try getting the semaphore for 10sec
        // similar to wait. It will wait till xSemaphoreGive is invoked from some other task
        if(xSemaphoreTake(onConnectionHandler, 10 * 1000 / portTICK_PERIOD_MS))
        {
            ESP_LOGI(TAG, "Processing...");
            // do something useful
            // e.g. connect to internet (once)

            struct FetchParams fetchParams;
            fetchParams.parseResponse = parseResponse;

            fetch("http://quotes.rest/qod", &fetchParams);
            ESP_LOGI(TAG, "%s", fetchParams.message);

            // have the data here and do something

            // Print the wifi connection info
            // These info are saved in flash
            // tcpip_adapter_ip_info_t ip_info;
            // ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
            // ESP_LOGI("IP Address: %s\n", ip4addr_ntoa(&ip_info.ip));
            // ESP_LOGI("Subnet Mask: %s\n", ip4addr_ntoa(&ip_info.netmask));

            // prevent this piece of code from running again
            // take the semaphore again. This will put the code in waiting state

            ESP_LOGI(TAG, "Done !");

            // Disconnect from the internet (save power)
            esp_wifi_disconnect();

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
    xTaskCreate(&onConnected, "On Connected", 1024*4, NULL, 5, NULL);
}