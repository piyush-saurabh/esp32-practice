#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h> // For checking how long ESP32 is running for
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h> // For activating sleep functions
#include <esp_log.h>
#include <esp32/rom/uart.h> // Used to include uart_tx_wait_idle()

// Light sleep with timer
void light_sleep_timer()
{
    // Enable light sleep timer
    // Specify the time to sleep in micro second
    esp_sleep_enable_timer_wakeup(5000000); // Sleep for 5 sec

    printf("going to light sleep\n");

    // Wait for the printf to complete before executing next line (before going to sleep)
    uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

    // Calculate the total time ESP32 has slept. For this calculate the before and after time
    // Get the total time (before) for which ESP32 is running for
    int64_t before = esp_timer_get_time();

    // Make ESP go to light sleep
    esp_light_sleep_start();

    // Get the total time (after) for which ESP32 is running for
    int64_t after = esp_timer_get_time();

    printf("Total sleep duration is %lld ms\n", (after - before) / 1000);
}

void light_sleep_gpio()
{

}

void deep_sleep_timer()
{

}

void deep_sleep_ext0()
{

}

void deep_sleep_ext1()
{

}

void app_main(void)
{
    light_sleep_timer();
}