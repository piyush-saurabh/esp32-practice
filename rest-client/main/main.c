#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h> // for connecting/disconnecting from wifi
#include <esp_log.h>

#include <cJSON.h> // JSON parsing library

// Rest client
#include "fetch.h"

// Connect to wifi
#include "connect.h"

// Binary semaphore for the task when wifi is connected
xSemaphoreHandle onConnectionHandler;

// Tag for logging
char *TAG = "MAIN";

// Parse the JSON response
// This logic will be specific to the URL
void parseResponse(char *incomingBuffer, char *output)
{
    cJSON *payload = cJSON_Parse(incomingBuffer);
    cJSON *contents = cJSON_GetObjectItem(payload, "contents");
    cJSON *quotes = cJSON_GetObjectItem(contents, "quotes");

    // Iterate over quotes array
    cJSON *quotesElement;
    cJSON_ArrayForEach(quotesElement, quotes)
    {
        cJSON *quote = cJSON_GetObjectItem(quotesElement, "quote");
        //ESP_LOGI(TAG,"%s",quote->valuestring);

        // do something with quote
        strcpy(output, quote->valuestring);
    }

    // clean cJSON
    cJSON_Delete(payload);
}

// Create the body for HTTP POST request
void createBody(char *number, char *message, char *out)
{
    // Alternative: use cJSON
    sprintf(out,
            "{"
            "  \"messages\": ["
            "      {"
            "      "
            "          \"content\": \"%s\","
            "          \"destination_number\": \"%s\","
            "          \"format\": \"SMS\""
            "      }"
            "  ]"
            "}",
            message, number);
}

// Task handler: Make HTTP request
void onConnected(void *param)
{
    while (true)
    {
        // If wifi is connected. Try getting the semaphore for 10sec
        // similar to wait. It will wait till xSemaphoreGive is invoked from some other task
        if (xSemaphoreTake(onConnectionHandler, 10 * 1000 / portTICK_PERIOD_MS))
        {
            ESP_LOGI(TAG, "Processing...");
            // do something useful
            // e.g. connect to internet (once)

            // Setting up the HTTP request parameters (method, headers and body)
            struct FetchParams fetchParams;
            fetchParams.parseResponse = parseResponse; // Function pointer for parsing the response
            fetchParams.body = NULL;                   // GET request will not have any request body
            fetchParams.headerCount = 0;
            fetchParams.method = GET; // GET Request

            fetch("http://quotes.rest/qod", &fetchParams);
            ESP_LOGI(TAG, "%s", fetchParams.message);

            // Check the request status of above request.
            // If the request is ok, send SMS (another HTTP Request)
            if (fetchParams.status == 200)
            {
                // Coonstruct the structure to send another HTTP Request (POST)
                // send sms
                struct FetchParams smsStruct;
                smsStruct.parseResponse = NULL;
                smsStruct.method = POST;

                // Defining HTTP request headers
                Header headerContentType = 
                {
                    .key = "Content-Type",
                    .val = "application/json"
                };

                Header headerAccept = 
                {
                    .key = "Accept",
                    .val = "application/json"
                };

                Header headerAuthorization = 
                {
                    .key = "Authorization",
                    .val = "Basic a3NScUxxOUZCeU9PbmVHVlJBSzA6aG5VMFJ5STVWcDJiRktSQWtIZEs5NmR6VnIzeTE3"
                };

                // Setting the HTTP headers
                smsStruct.header[0] = headerAuthorization;
                smsStruct.header[1] = headerAccept;
                smsStruct.header[2] = headerContentType;

                // Set the header count. This is required in fetch.c while setting up the header
                smsStruct.headerCount = 3;

                // POST request body
                char buffer[1024];

                createBody("+1234567890", fetchParams.message, buffer);
                smsStruct.body = buffer;
                fetch("https://api.messagemedia.com/v1/messages", &smsStruct);
            }

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
    xTaskCreate(&onConnected, "On Connected", 1024 * 5, NULL, 5, NULL);
}