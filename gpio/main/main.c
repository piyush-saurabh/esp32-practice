#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define the pin number
// Onboard LED is connected to this pin
#define PIN 2

// Input Pin
#define PUSH_BUTTON_PIN 4

// External LED Pin (OUTPUT)
#define LED_PIN 17

// GPIO Output
void gpio_output_demo()
{
    // Select the GPIO
    gpio_pad_select_gpio(PIN);

    // Define whether the Pin should be input or output
    gpio_set_direction(PIN, GPIO_MODE_OUTPUT);

    int isOn = 0;
    while (true)
    {
        // Toggle the Pin
        isOn = !isOn;

        // Set the value of a GPIO pin
        gpio_set_level(PIN, isOn);

        // Wait for 1 sec
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// GPIO Input (push button)
// On push of a button, turn the external LED on/off
void gpio_input_demo()
{
    // Select the push button GPIO and set it as input
    gpio_pad_select_gpio(PUSH_BUTTON_PIN);
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);

    // Select the external LED pin and set it as output
    gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // Use internal pulldown register for the push button
    // Now no external pulldown register is required
    gpio_pulldown_en(PUSH_BUTTON_PIN);

    // After enabling pullup register, disable the pulldown register. Not disabling this can cause issue
    gpio_pullup_dis(PUSH_BUTTON_PIN);

    // enable pullup register
    // gpio_pullup_en(PUSH_BUTTON_PIN);

    while (true)
    {
        // Get the state of the pin connected to push button
        int level = gpio_get_level(PUSH_BUTTON_PIN);

        printf("Button pressed with level: %d\n", level);

        // Turn on the external LED
        gpio_set_level(LED_PIN, level);

        // Prevent the watch dog timer from returning error (infinite loop)
        vTaskDelay(1);
    }
}

void app_main(void)
{

    //gpio_output_demo();
    gpio_input_demo();
}

/*

voltage drop across LED = 2.75V
Max current that can flow through a GPIO = 0.02 A
Resistance required = 2.75/0.02 = 30 OHM

*/
