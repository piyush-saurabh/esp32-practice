#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


void led_control(void *param)
{
    while(1)
    {
        printf("controlling the led ...\n");
        
        // wait for 2sec
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}