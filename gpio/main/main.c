#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Define the pin number
// Onboard LED is connected to this pin
#define PIN 2

// Input Pin
#define PUSH_BUTTON_PIN 4

// External LED Pin (OUTPUT)
#define LED_PIN 17

// Queue
xQueueHandle interputQueue;

// ISR handler for button press
// IRAM_ATTR is a compiler directive that tells c compiler to run this method from Dynamic RAM (DRAM) rather than Standard RAM (SRAM)
// Keep ISR short
static void IRAM_ATTR gpio_isr_handler(void *args)
{
    // Get the pin number passed as an argument
    int pinNumber = (int)args;

    // Return to the normal code flow by create a new FreeRTOS task and invoking it.
    // Add the pin to the queue (there might be multiple push buttons on the device)
    xQueueSendFromISR(interputQueue, &pinNumber, NULL);
}

// Task which keeps on running in background
void buttonPushedTask(void *params)
{
    int pinNumber, count = 0;
    while (true)
    {
        // Wait for the queue to get triggered
        if (xQueueReceive(interputQueue, &pinNumber, portMAX_DELAY))
        {
            /*
            // Debouncing technique
            1. Disable the interrupt
            2. Wait for sometime to check for the button to be released
            3. Do some work
            4. Re-enable the interrupt
            */

            // 1. Disable the interrupt
            gpio_isr_handler_remove(pinNumber);

            // 2. wait for sometime for the debouncing to stabalize
            do
            {
                // wait for 50 ms
                // Fine tune it based on the button in use
                vTaskDelay(50 / portTICK_PERIOD_MS);
            } while (gpio_get_level(pinNumber) == 1);

            printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(PUSH_BUTTON_PIN));

            // re-enable the interrupt
            gpio_isr_handler_add(pinNumber, gpio_isr_handler, (void *)pinNumber);
        }
    }
}

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
    // UNCOMMENT if not using external pulldown register
    //gpio_pulldown_en(PUSH_BUTTON_PIN);

    // After enabling pullup register, disable the pulldown register. Not disabling this can cause issue
    // UNCOMMENT if not using external pulldown register
    // gpio_pullup_dis(PUSH_BUTTON_PIN);

    // enable pullup register
    // gpio_pullup_en(PUSH_BUTTON_PIN);

    // Interrupt configuration
    // Wake up on positive edge because we have used pulldown register (default 0, button press 1)
    gpio_set_intr_type(PUSH_BUTTON_PIN, GPIO_PIN_INTR_POSEDGE); 

    // Interrupt service
    gpio_install_isr_service(0);

    // Add interrupt service to our pin
    // create a callback function and pass the pin number as the parameter to the handler
    gpio_isr_handler_add(PUSH_BUTTON_PIN, gpio_isr_handler, (void *)PUSH_BUTTON_PIN);

    /*
    // This loop waits for button press which consumes CPU cycle
    // Refactor this while loop to interrupt
    while (true)
    {
        // Get the state of the pin connected to push button
        // Level will be 0 or 1
        int level = gpio_get_level(PUSH_BUTTON_PIN);

        printf("Button pressed with level: %d\n", level);

        // Turn on the external LED
        gpio_set_level(LED_PIN, level);

        // Prevent the watch dog timer from returning error (infinite loop)
        vTaskDelay(1);
    }
    */
}

void app_main(void)
{

    //gpio_output_demo();

    // Create a queue for handling interrupt tasks
    interputQueue = xQueueCreate(10, sizeof(int));

    // Create a task that can run in background
    xTaskCreate(buttonPushedTask, "buttonPushedTask", 2048, NULL, 1, NULL);

    gpio_input_demo();
}

/*

voltage drop across LED = 2.75V
Max current that can flow through a GPIO = 0.02 A
Resistance required = 2.75/0.02 = 30 OHM

*/
