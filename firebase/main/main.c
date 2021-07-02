#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h> // for connecting/disconnecting from wifi
#include <esp_log.h>

#include <cJSON.h> // JSON parsing library

// Rest client for making http request
//#include "http_connect.h"

// For making http stream request
#include "http_stream.h"

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

// Firebase RealTime Database Certificate
extern const uint8_t realtimedb_root_cert_pem_start[] asm("_binary_realtimedb_pem_start");

/*
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

*/

// Task to read from RealTime Database
void firebase_realtime_db(void *param)
{
    while(true)
    {
        if (xSemaphoreTake(wifiConnectHandler, 10 * 1000 / portTICK_PERIOD_MS))
        {
            char *realtimeDbURL = "https://iot-chip2cloud.firebaseio.com/users/3mWUE2leYBfHKBTxCWlIEyPTYzg1/test.json?auth=eyJhbGciOiJSUzI1NiIsImtpZCI6IjMwMjUxYWIxYTJmYzFkMzllNDMwMWNhYjc1OTZkNDQ5ZDgwNDI1ZjYiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL3NlY3VyZXRva2VuLmdvb2dsZS5jb20vaW90LWNoaXAyY2xvdWQiLCJhdWQiOiJpb3QtY2hpcDJjbG91ZCIsImF1dGhfdGltZSI6MTYyMTY5Njk5NSwidXNlcl9pZCI6IjNtV1VFMmxlWUJmSEtCVHhDV2xJRXlQVFl6ZzEiLCJzdWIiOiIzbVdVRTJsZVlCZkhLQlR4Q1dsSUV5UFRZemcxIiwiaWF0IjoxNjIyMzA3MDA5LCJleHAiOjE2MjIzMTA2MDksImVtYWlsIjoiY2hpcDJjbG91ZEByb2d1ZXNlY3VyaXR5LmluIiwiZW1haWxfdmVyaWZpZWQiOmZhbHNlLCJmaXJlYmFzZSI6eyJpZGVudGl0aWVzIjp7ImVtYWlsIjpbImNoaXAyY2xvdWRAcm9ndWVzZWN1cml0eS5pbiJdfSwic2lnbl9pbl9wcm92aWRlciI6InBhc3N3b3JkIn19.FW0ynx7J_Se7Tyk3uEc0ieoJWZIjVkABi5lcbjlKLx_OokS0XvzSCek-AbDPXix7Mu3G8kYLgSojilVE7r2c1BGQVHRwZ-h4aT9AzvaoHsEawScURx2EB978ye0yrVrNaE5LITnLCZbeOdK3U3m-Kv_2sbebvQJYnRpBS2FQ9te9JtYzbxkEDzNFCOyTrT7nIrsn8DgiyVL6NTWcP2FERNs57E97yktigpdIKNrcrNUeehFpEF1yzqvxpYBAhvVSsNcWC6MCftX9SNYokqpXFiyNIt5_JYUXz6wZwFMwtO_Ty3zIMA_fZ4yMJITc822ZYQxFpRTlg3XJV-xyPdTu1w";

            // Setting up the HTTP request parameters (method, headers and body)
            struct HttpConnectParams realTimeDbParams;

            // Function pointer for parsing the response
            //realTimeDbParams.http_response_parser = parse_auth_response;                
            
            realTimeDbParams.method = GET;

            // Defining HTTP request headers
            realTimeDbParams.headerCount = 1;
            Header headerContentType =
                {
                    .key = "Accept",
                    .val = "text/event-stream"};

            realTimeDbParams.header[0] = headerContentType;

            // Root server cert for ssl verification
            realTimeDbParams.serverCert = realtimedb_root_cert_pem_start;

            // Send the http request
            open_http_stream(realtimeDbURL, &realTimeDbParams);


            ESP_LOGI(TAG, "Done");

            xSemaphoreTake(wifiConnectHandler, portMAX_DELAY);
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
    //xTaskCreate(&firebase_auth, "Firebase", 1024 * 7, NULL, 1, NULL);

    // Open connection with Real Time database
    xTaskCreate(&firebase_realtime_db, "RealTimeDB", 1024 * 15, NULL, 1, NULL);


}