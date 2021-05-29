#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h> // for connecting/disconnecting from wifi
#include <esp_log.h>

#include <cJSON.h> // JSON parsing library

// Rest client for making http request
#include "http_connect.h"

// Connect to wifi
#include "wifi_connect.h"

// Comment it to stop debugging
#define DEBUG

// Binary semaphore for the task when wifi is connected
xSemaphoreHandle wifiConnectHandler;

// Tag for logging
char *TAG = "MAIN";

// Firebase Auth Server Certificate
extern const uint8_t auth_root_cert_pem_start[] asm("_binary_auth_pem_start");

// Parse the JSON response
// This logic will be specific to the URL
void parse_auth_response(char *incomingBuffer, char *output)
{
    cJSON *payload = cJSON_Parse(incomingBuffer);
    cJSON *idToken = cJSON_GetObjectItem(payload, "idToken");
    //cJSON *expiresIn = cJSON_GetObjectItem(contents, "expiresIn");

    //ESP_LOGI(TAG, "ID Token fetched successfully %s", incomingBuffer);

    //ESP_LOGI(TAG, "ID Token is %s\n", idToken->valuestring);

    // Copy the id token into the output
    strcpy(output, idToken->valuestring);

    // clean cJSON
    cJSON_Delete(payload);
}

// Create the body for HTTP POST request
void create_auth_request_body(char *email, char *password, char *out)
{
    #ifdef DEBUG
    ESP_LOGD(TAG, "Creating the request body. Email: %s; Password: %s\n", email, password);
    #endif

    sprintf(out, 
        "{"
        "   \"email\": \"%s\","
        "   \"password\": \"%s\","
        "   \"returnSecureToken\": true"
        "}", email, password);

}

// Task to fetch the firebase auth token
void firebase_auth(void *param)
{
    while (true)
    {
        // If wifi is connected. Try getting the semaphore for 10sec
        // similar to wait. It will wait till xSemaphoreGive is invoked from some other task (wifi handler)
        if (xSemaphoreTake(wifiConnectHandler, 10 * 1000 / portTICK_PERIOD_MS))
        {
            char firebaseAuthEndpoint[256];
            char *url = "%s?key=%s";

            sprintf(firebaseAuthEndpoint, url, CONFIG_FIREBASE_AUTH_URL, CONFIG_FIREBASE_API_KEY);

            #ifdef DEBUG
            ESP_LOGI(TAG, "Connecting to %s for authentication...\n", firebaseAuthEndpoint);
            #endif

            // Setting up the HTTP request parameters (method, headers and body)
            struct HttpConnectParams firebaseAuthParams;

            // Function pointer for parsing the response
            firebaseAuthParams.http_response_parser = parse_auth_response;                
            
            firebaseAuthParams.method = POST;

            // Defining HTTP request headers
            firebaseAuthParams.headerCount = 1;
            Header headerContentType =
                {
                    .key = "Content-Type",
                    .val = "application/json"};

            firebaseAuthParams.header[0] = headerContentType;

            // Create the request body
            char buffer[512];
            create_auth_request_body(CONFIG_FIREBASE_EMAIL, CONFIG_FIREBASE_PASSWORD, buffer);
            firebaseAuthParams.body = buffer;

            // Root server cert for ssl verification
            firebaseAuthParams.serverCert = auth_root_cert_pem_start;

            // Send the http request
            send_http_request(firebaseAuthEndpoint, &firebaseAuthParams);

            // Check the request status of above request.
            if (firebaseAuthParams.status == 200)
            {
                ESP_LOGI(TAG, "200 response received\n");

                ESP_LOGI(TAG, "Received ID Token is %s\n", firebaseAuthParams.parsedResponse);
            }

            ESP_LOGI(TAG, "Done");

            xSemaphoreTake(wifiConnectHandler, portMAX_DELAY);
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
    wifiConnectHandler = xSemaphoreCreateBinary();

    // Initialize the wifi
    wifiInit();

    // create a task for making HTTP request
    xTaskCreate(&firebase_auth, "Firebase", 1024 * 7, NULL, 1, NULL);
}