#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include <battery_monitor.h>
#include <mosfet_switch.h>

#define TAG "MAIN"

// Comment it to stop debugging
#define DEBUG

void app_main()
{
#ifdef DEBUG
    ESP_LOGI(TAG, "Started...");
#endif

    // Start the temperature sensor task
    xTaskCreate(&battery_monitor, "battery_monitor", 2048, NULL, 1, NULL);

    // Start the led control task
    xTaskCreate(&mosfet_switch, "mosfet_switch", 2048, NULL, 1, NULL);
}