#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_http_client.h>
#include <esp_log.h>

#include "http_connect.h"

#define TAG "HTTP_CONNECT"

// Comment it to stop debugging
#define DEBUG

//#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048 // Buffer size of the total HTTP response. Change it if the http response size is larger
#define CHUNK_BUFFER_SIZE 700 // Buffer size of the HTTP response received as chunk 


// buffer for storing response data (dynamic memory allocation)
char chunk_buffer[CHUNK_BUFFER_SIZE] = {0};
char response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};

// Event handler for processing the response made by the http client
// TODO implement all the cases here https://github.com/espressif/esp-idf/blob/master/examples/protocols/esp_http_client/main/esp_http_client_example.c#L55

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    struct HttpConnectParams *httpConnectParams = (struct HttpConnectParams *)evt->user_data;

    switch (evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA Len=%d", evt->data_len);

            //printf("%.*s\n", evt->data_len, (char *)evt->data);
            //printf("lenght of data = %d\n", strlen((char *)evt->data));
            sprintf(chunk_buffer, "%.*s", evt->data_len, (char *)evt->data);
            strcat(response_buffer, chunk_buffer);
            break;

        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");

            //printf("Response: %s\n", response_buffer);

            // Parse the JSON by invoking callback method
            // Make sure that the callback method is set
            if (httpConnectParams->http_response_parser != NULL)
            {
                //fetchParams->parseResponse(buffer, fetchParams->message);
                httpConnectParams->http_response_parser(response_buffer, httpConnectParams->parsedResponse);
            }
            break;

        case HTTP_EVENT_DISCONNECTED:
                ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
                break;

        default:
            break;

    }

    return ESP_OK;
}

void send_http_request(char *url, struct HttpConnectParams *httpConnectParams)
{
    #ifdef DEBUG
    ESP_LOGI(TAG, "Sending HTTP Request with url: %s, method: %d, header:%s, body: %s\n", url, httpConnectParams->method, httpConnectParams->header[0].key, httpConnectParams->body);
    #endif

    // HTTP Client configuration
    const esp_http_client_config_t clientConfig = {
        .url = url,
        .cert_pem = (char *)httpConnectParams->serverCert,
        .event_handler = http_event_handler,
        .user_data = httpConnectParams};

    // Initialize the HTTP client
    esp_http_client_handle_t client = esp_http_client_init(&clientConfig);

    // Set the appropriate HTTP method
    switch (httpConnectParams->method)
    {
    case GET:
        esp_http_client_set_method(client, HTTP_METHOD_GET);
        break;
    case POST:
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        break;
    default:
        esp_http_client_set_method(client, HTTP_METHOD_GET);
        break;
    }

    // Set the headers
    for (int i = 0; i < httpConnectParams->headerCount; i++)
    {
        esp_http_client_set_header(client, httpConnectParams->header[i].key, httpConnectParams->header[i].val);
    }

    // Set the body
    if (httpConnectParams->body != NULL)
    {
        esp_http_client_set_post_field(client, httpConnectParams->body, strlen(httpConnectParams->body));
    }

    // Make HTTP request
    esp_err_t err = esp_http_client_perform(client);

    // Update the response status
    httpConnectParams->status = esp_http_client_get_status_code(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}