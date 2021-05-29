#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_event_loop.h>
#include <esp_wifi.h>  // for connecting to the internet
#include <esp_log.h>
#include <nvs_flash.h> // used by wifi internally to store information

#define TAG "WIFI_CONNECT"

// SSID and password for connecting to Wifi
// These configuration will be fetched from Kconfig.projbuild
// #define SSID CONFIG_WIFI_SSID
// #define PASSWORD CONFIG_WIFI_PASSWORD

// Binary semaphore for the task when wifi is connected
extern xSemaphoreHandle wifiConnectHandler;


// Event loop handler for WiFi
static void event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
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
        //ESP_LOGI("stack space is %d\n", uxTaskGetStackHighWaterMark(NULL)); // 1008 bytes; not enough space to make HTTP request, so we will do this in separate task firebase_connect()

        // invoke a FreeRTOS task after connecting to wifi
        xSemaphoreGive(wifiConnectHandler);
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "disconnected\n");
        break;

    default:
        break;
    }
    //return ESP_OK;
}


// Initialize the wifi (reusable code)
void wifiInit()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    // initialize TCP adapter
    ESP_ERROR_CHECK(esp_netif_init());
    // tcpip_adapter_init(); // depricated


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
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL));

    // Event handler for IP
    // Fire the event handler on receiving the IP
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL));

    // Store ALL the wifi configuration in RAM
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Use WiFi as a Station mode (connect to a router)
    // scan for other wifi AP near by
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); //depricated

    // Connect to a wifi as station
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD}};

    // configure the wifi to use above configuration
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    // start the wifi within the chip
    ESP_ERROR_CHECK(esp_wifi_start());
}