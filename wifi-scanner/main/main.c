#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h>  // for connecting to the internet
#include <esp_event.h> // wifi uses event loop to act on the events associated with wifi
#include <nvs_flash.h> // used by wifi internally to store information
#include <esp_http_client.h>
#include <esp_log.h>
#include <esp_event_loop.h>

// Max number of scan results we want to display
#define MAX_APs 20

// Get different authentication mode for wifi (used to display scan result)
static char *getAuthModeName(wifi_auth_mode_t auth_mode)
{
  char *names[] = {"OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "MAX"};
  return names[auth_mode];
}

// Event loop handler for WiFi
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

// Initialize the wifi (reusable code)
void wifiInit()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    // initialize TCP adapter
    tcpip_adapter_init();

    // Event Loop
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    // configure the wifi settings
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();

    // Initialize ESP with the above wifi configuration
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));

    // Use WiFi as a Station mode (connect to a router)
    // scan for other wifi AP near by
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // start the wifi within the chip
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void)
{
    // Initialize the wifi
    wifiInit();

    // wifi scanning configuration
    wifi_scan_config_t scan_config = {
        .ssid = 0,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true
    };

    // Start scanning the near by wifi networks
    // pause the program execution (true) till the scan is complete
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

    // Get the results of wifi scan
    wifi_ap_record_t wifi_records[MAX_APs];

    // Fill the above structure with the wifi scan result
    uint16_t maxRecods = MAX_APs;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&maxRecods, wifi_records));

    printf("Found %d access points:\n", maxRecods);
    printf("\n");
    printf("               SSID              | Channel | RSSI |   Auth Mode \n");
    printf("----------------------------------------------------------------\n");
    for (int i = 0; i < maxRecods; i++)
        printf("%32s | %7d | %4d | %12s\n", (char *)wifi_records[i].ssid, wifi_records[i].primary, wifi_records[i].rssi, getAuthModeName(wifi_records[i].authmode));
    printf("----------------------------------------------------------------\n");
}