#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/semphr.h>

#include<temperature_sensor.h>
#include<led_control.h>
#include<interrupt_task.h>


// Dummy 3.3 V source
#define DUMMY_VCC_PIN 23

extern xSemaphoreHandle pushButtonSemaphore;


 // Use GPIO as 3.3V source (since there is only 1 3.3V pin on ESP32 board)
void set_dummy_voltage_source()
{
    gpio_pad_select_gpio(DUMMY_VCC_PIN);
    gpio_set_direction(DUMMY_VCC_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DUMMY_VCC_PIN, 1);
}

void app_main(void)
{
    // Create the semaphore
    pushButtonSemaphore = xSemaphoreCreateBinary();
    interputQueue = xQueueCreate(10, sizeof(int));

    set_dummy_voltage_source();
    
    // Start the temperature sensor task
    xTaskCreate(&temperature_sensor, "temp_sensor", 2048, NULL, 1, NULL);

    // Start the led control task
    xTaskCreate(&led_control, "led", 2048, NULL, 1, NULL);

    // Start the interrupt task
    xTaskCreate(&interrupt_task, "interrupt", 2048, NULL, 1, NULL);

}