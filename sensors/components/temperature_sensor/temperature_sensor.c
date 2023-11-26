#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "dht.h"
#include <driver/gpio.h>
#include<sd_card.h>


#define TAG "TEMPERATURE_SENSOR"

//#define SENSOR_PIN 15

void temperature_sensor(void *param)
{
    int16_t humidity;
    int16_t temperature;
    gpio_num_t sensorPin = 33;

    char *log_file_name = "/store/log-3.csv";
    //log_to_sdcard(log_file_name, "Time,Temperature,Humidity\n");

    log_to_sdcard(log_file_name, "Time,Temperature,Humidity\n");
    

    printf("The value of gpio %d is %d\n", sensorPin, gpio_get_level(sensorPin));

    //gpio_set_pull_mode(sensorPin, GPIO_PULLUP_ONLY);

    // printf("Changing the pin level to LOW...\n");
    // gpio_pad_select_gpio(sensorPin);
    // gpio_set_direction(sensorPin, GPIO_MODE_OUTPUT);
    // gpio_set_level(sensorPin, 1);
    // vTaskDelay(5000 / portTICK_PERIOD_MS);
    // printf("New value of gpio %d is %d\n", sensorPin, gpio_get_level(sensorPin));

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

            // Log to SD Card
            char buffer[100];
            sprintf(buffer, "%lld,%d,%d\n", esp_timer_get_time(), temperature, humidity);
            log_to_sdcard(log_file_name, buffer);
             
        }

        // wait for 10 mins
        vTaskDelay(60000 * 10 / portTICK_PERIOD_MS);
    }
}