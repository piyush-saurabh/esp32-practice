#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_http_client.h>
#include <esp_log.h>
//#include <cJSON.h> // JSON parsing library

#include "fetch.h"
#include "connect.h"

#define TAG "HTTPCLIENT"

// buffer for storing response data (dynamic memory allocation)
char *buffer = NULL;
int buffer_index = 0;

// parse the JSON
//void parseResponse(char *incomingBuffer);

// Event handler for processing the response made by the http client
esp_err_t clientEventHandler(esp_http_client_event_t *evt)
{
    struct FetchParams *fetchParams = (struct FetchParams *)evt->user_data;

    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA Len=%d", evt->data_len);

        // the buffer size is limited, so we will get data in multiple loops. To solve this, 1st store all the data in buffer and display it at the end
        //printf("%.*s\n", evt->data_len, (char *)evt->data);  

        // solution
        if (buffer == NULL)
        {
            // 1st time we are allocating to buffer
            buffer = (char *)malloc(evt->data_len);
        }
        else
        {
            // some memory is already allocated to buffer
            buffer = (char *)realloc(buffer, evt->data_len + buffer_index);

        }

        // copy the incoming response into buffer
        memcpy(&buffer[buffer_index], evt->data, evt->data_len);
        buffer_index += evt->data_len;

        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");

        // add 1 more byte to the buffer to make it string
        buffer = (char *)realloc(buffer, buffer_index+1);
        memcpy(&buffer[buffer_index+1], "\0", 1);
        ESP_LOGI(TAG, "%s", buffer);

        // Parse the JSON
        fetchParams->parseResponse(buffer, fetchParams->message);

        // clear the buffer
        free(buffer);
        buffer = NULL;
        buffer_index = 0;

        break;

    default:
        break;
    }

    return ESP_OK;
}



void fetch(char *url, struct FetchParams *fetchParams)
{
    // HTTP Client configuration
    esp_http_client_config_t clientConfig = {
        .url = url,
        .event_handler = clientEventHandler,
        .user_data = fetchParams
    };

    // Initialize the HTTP client
    esp_http_client_handle_t client = esp_http_client_init(&clientConfig);

    // Make HTTP request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP GET status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);

}