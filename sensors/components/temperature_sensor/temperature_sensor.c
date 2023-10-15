#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "dht.h"


#define TAG "TEMPERATURE_SENSOR"

#define SENSOR_PIN 33

void temperature_sensor(void *param)
{
    int16_t humidity;
    int16_t temperature;
    gpio_num_t sensorPin = 33;

    while(1)
    {
        printf("Reading the temperature...\n");

        esp_err_t error = dht_read_data(DHT_TYPE_DHT11, sensorPin, &humidity, &temperature);

        if(error == ESP_OK)
        {
            // Output is integer
            // E.g. humidity=625 is 62.5 %, temperature=244 is 24.4 degrees Celsius
            humidity = humidity / 10;
            temperature = temperature / 10;

            printf("Temperature = %d °C\nHumidity = %d %%\n\n", temperature, humidity);
        }

        // wait for 5 sec
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}