#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#define TAG "TEMPERATURE_SENSOR"

void temperature_sensor(void *param)
{
    while(1)
    {
        printf("Calculating the temperature...\n");

        // wait for 1sec
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}