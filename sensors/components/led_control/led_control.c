#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

// WiFi Status LED
#define RED_LED_PIN 4

// Internet Status LED
#define GREEN_LED_PIN 25

void change_led_state(int pin, int state)
{
    gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, state);
}

void led_control(void *param)
{
    while(1)
    {
        printf("turning on the RED led ...\n");
        change_led_state(RED_LED_PIN, 1);
        vTaskDelay(2000 / portTICK_PERIOD_MS); // wait for 2 sec

        printf("turning on the GREEN led ...\n");
        change_led_state(GREEN_LED_PIN, 1);
        vTaskDelay(2000 / portTICK_PERIOD_MS); // wait for 2 sec

        printf("turning off the RED led ...\n");
        change_led_state(RED_LED_PIN, 0);
        vTaskDelay(2000 / portTICK_PERIOD_MS); // wait for 2 sec

        printf("turning off the GREEN led ...\n");
        change_led_state(GREEN_LED_PIN, 0);
        vTaskDelay(2000 / portTICK_PERIOD_MS); // wait for 2 sec


    }
}