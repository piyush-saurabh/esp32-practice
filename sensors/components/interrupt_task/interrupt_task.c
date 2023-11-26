#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_timer.h>
#include <driver/gpio.h>


#define TAG "INTERRUPT_TASK"

#define DEBOUNCE_TIME 50 // Debounce time in ms
#define PUSH_BUTTON_PIN 2



// Semaphore for handling push button
xSemaphoreHandle pushButtonSemaphore;

// ISR handler for button press or when the GPIO Pin receives a signal
static void IRAM_ATTR gpio_isr_handler(void *args)
{
    // Get the pin number passed as an argument
    //int pinNumber = (int)args;

    //NEVER PRINT INSIDE ISR !!!!!! This will cause the crash and restart of ESP

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(pushButtonSemaphore, &xHigherPriorityTaskWoken);

}

// Initialize the button
void interrupt_gpio_init()
{
    // Select the push button GPIO and set it as input
    gpio_pad_select_gpio(PUSH_BUTTON_PIN);
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(PUSH_BUTTON_PIN);
    

    // Interrupt configuration
    // Wake up on positive edge because we have used pulldown register (default 0, button press 1)
    gpio_set_intr_type(PUSH_BUTTON_PIN, GPIO_PIN_INTR_POSEDGE); 

    // Interrupt service
    gpio_install_isr_service(0);

    // Add interrupt service to our pin
    // create a callback function and pass the pin number as the parameter to the handler
    gpio_isr_handler_add(PUSH_BUTTON_PIN, gpio_isr_handler, (void *)PUSH_BUTTON_PIN);
}

void interrupt_task(void *params)
{
    // The interrupt GPIO is pulled down by default
    interrupt_gpio_init();

    int pinNumber, count = 0;
    pinNumber = PUSH_BUTTON_PIN;

 while (1)
    {
        // Wait for the queue to get triggered
        //if (xQueueReceive(interputQueue, &pinNumber, portMAX_DELAY))
        if (xSemaphoreTake(pushButtonSemaphore, portMAX_DELAY))
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
                // wait for DEBOUNCE time in ms
                // Fine tune it based on the button in use
                vTaskDelay(DEBOUNCE_TIME / portTICK_PERIOD_MS);
            } while (gpio_get_level(pinNumber) == 1);

            printf("GPIO %d was pressed %d times. The value of gpio is %d\n", pinNumber, count++, gpio_get_level(PUSH_BUTTON_PIN));

            // re-enable the interrupt
            gpio_isr_handler_add(pinNumber, gpio_isr_handler, (void *)pinNumber);
        }
    }

}
