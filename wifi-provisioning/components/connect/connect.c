#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_event_loop.h>
#include <esp_wifi.h> // for connecting to the internet
#include <esp_log.h>
#include <nvs_flash.h> // used by wifi internally to store information

#include "connect.h"

#define TAG "CONNECT"

// SSID and password for connecting to Wifi
// These configuration will be fetched from Kconfig.projbuild
#define SSID CONFIG_WIFI_SSID
#define PASSWORD CONFIG_WIFI_PASSWORD

// Binary semaphore for the task when wifi is connected
extern xSemaphoreHandle connectionSemaphore;

// Event loop handler for WiFi STA
static void sta_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG, "connecting...\n");
        break;

    case SYSTEM_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "connected\n");
        break;

    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip\n");
        //ESP_LOGI("stack space is %d\n", uxTaskGetStackHighWaterMark(NULL)); // 1008 bytes; not enough space to make HTTP request, so we will do this in separate task onConnected()
        // invoke a FreeRTOS task after connecting to wifi
        //xSemaphoreGive(connectionSemaphore);
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "disconnected\n");
        break;

    default:
        break;
    }
    //return ESP_OK;
}

// Event loop handler for WiFi AP
static void ap_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

// Wifi Configuration for starting the chip in Station Mode
void connectSTA(char *ssid, char *password)
{
    // initialize TCP adapter
    ESP_ERROR_CHECK(esp_netif_init());

    // Event Loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL)); // depricated

    // Use WiFi as a Station mode (connect to a router)
    // scan for other wifi AP near by
    esp_netif_create_default_wifi_sta();

    // configure the wifi settings
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

    // Initialize ESP with the above wifi configuration
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    // Register the event handler for wifi
    // Call the event handler function on any wifi event id (connection, disconnection etc)
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, sta_event_handler, NULL));

    // Event handler for IP
    // Fire the event handler on receiving the IP
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_event_handler, NULL));

    // Store ALL the wifi configuration in RAM
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Use WiFi as a Station mode (connect to a router)
    // scan for other wifi AP near by
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); //depricated

    // Connect to a wifi as station

    // Cannot be used to dynamically set the creds
    // wifi_config_t wifi_config = {
    //     .sta = {
    //         .ssid = CONFIG_WIFI_SSID,
    //         .password = CONFIG_WIFI_PASSWORD}};

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));

    // TODO fix buffer overflow
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);

    // configure the wifi to use above configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    // start the wifi within the chip
    ESP_ERROR_CHECK(esp_wifi_start());

    // TODO If not able to connect to wifi for any reason (wifi not reachable, wrong ssid/password) start clean the NVS and restart the chip
}

// Wifi configuration for starting the chip in Access Point Mode
void connectAP()
{
    // initialize TCP adapter
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Configure Wifi to AP mode
    esp_netif_create_default_wifi_ap();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(TAG,
                                                        ESP_EVENT_ANY_ID,
                                                        &ap_event_handler,
                                                        NULL,
                                                        NULL));

    // Initialize the AP
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // AP configuration settings
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK}};

    // configure the wifi to use above configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

    // start the wifi within the chip
    ESP_ERROR_CHECK(esp_wifi_start());
}

// Initialize the wifi (reusable code)
// This is a FreeRTOS task
void wifiInit(void *params)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    // Task should always loop
    while (true)
    {
        // Wait using semaphore
        if (xSemaphoreTake(initSemaphore, portMAX_DELAY))
        {
            // Check if wifi is already provisioned
            // Check if the wifi credentials are configured in NVS
            nvs_handle_t nvs;
            nvs_open("wifiCreds", NVS_READWRITE, &nvs);

            // Check the size of ssid and password
            size_t ssidLen, passLen;
            char *ssid = NULL, *pass = NULL;

            // TODO Refactor this code as there are duplication in logic for ssid and password
            // Check if there is any SSID key is nvs
            // Passing NULL will return the lenght of ssid back
            if (nvs_get_str(nvs, "ssid", NULL, &ssidLen) == ESP_OK)
            {
                // If ssid is set
                // Assign some space in memory for ssid
                if (ssidLen > 0)
                {
                    ssid = malloc(ssidLen);

                    // Fetch the ssid from the NVS
                    nvs_get_str(nvs, "ssid", ssid, &ssidLen);
                }
            }

            // Check if there is any pass key is nvs
            if (nvs_get_str(nvs, "pass", NULL, &passLen) == ESP_OK)
            {
                // If pass is set
                // Assign some space in memory for password
                if (passLen > 0)
                {
                    pass = malloc(passLen);

                    // Fetch the password from the NVS
                    nvs_get_str(nvs, "pass", pass, &passLen);
                }
            }

            // If we already have SSID and password, start the chip in STA mode
            if (ssid != NULL && pass != NULL)
            {
                connectSTA(ssid, pass);
            }
            else
            {
                // Start the chip in AP mode
                connectAP();
            }

            // invoke a FreeRTOS task after connecting to wifi / starting as AP
            xSemaphoreGive(connectionSemaphore);

            // Memory cleanup
            if (ssid != NULL)
                free(ssid);
            if (pass != NULL)
                free(pass);
        }
    }
}