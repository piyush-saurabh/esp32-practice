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
#include <driver/rtc_io.h> // This allows to use GPIO pins while sleeping  

// Use Boot button to wake up from light sleep
#define INPUT_PIN 0

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

// Wake up from light sleep by clicking a button
void light_sleep_timer_gpio()
{
    // Configure the GPIO pin as input (button)
    gpio_pad_select_gpio(INPUT_PIN);
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);

    // Setup the wakeup interrupt (not for wakeup)
    // This will be triggered when button is released
    gpio_wakeup_enable(INPUT_PIN, GPIO_INTR_LOW_LEVEL);

    // Enable the GPIO
    esp_sleep_enable_gpio_wakeup();

    // Configure light sleep for 5 sec
    esp_sleep_enable_timer_wakeup(5000000);

    // Loop so that the program the process of sleep and wakeup continues
    while (true)
    {
        // Halt the application untill the button is released
        // rtc_gpio_get_level() has the capability to read the pins during sleep
        if (rtc_gpio_get_level(INPUT_PIN) == 0)
        {
            printf("Please release the button\n");
            do
            {
                // wait for 10ms
                vTaskDelay(pdMS_TO_TICKS(10));
            } while (rtc_gpio_get_level(INPUT_PIN) == 0);
        }

        printf("Starting light sleep\n");
        uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

        int64_t before = esp_timer_get_time();

        esp_light_sleep_start();

        int64_t after = esp_timer_get_time();

        // Cause of wakeup
        // It can be button press or timer
        esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

        printf("Light Sleep duration is for %lld ms, reason was %s\n", (after - before) / 1000,
               reason == ESP_SLEEP_WAKEUP_TIMER ? "timer" : "button");
    }



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
    // demo for light sleep only with timer
    //light_sleep_timer();

    // demo for light sleep with gpio and timer
    light_sleep_timer_gpio();
}