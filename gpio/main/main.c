#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/dac.h" // for digital to analog conversion
#include "driver/adc.h" // for analog to digital conversion
#include "driver/ledc.h" // for pwm

// Define the pin number
// Onboard LED is connected to this pin
#define PIN 2

// Input Pin
#define PUSH_BUTTON_PIN 4

// External LED Pin (OUTPUT)
#define LED_PIN 17

// Dummy 3.3 V source
#define DUMMY_VCC_PIN 23


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

// GPIO input with configuration
void gpio_config_demo()
{
    gpio_config_t config;

    config.intr_type = GPIO_INTR_POSEDGE;
    config.mode = GPIO_MODE_INPUT;
    //config.pull_down_en = true;
    //config.pull_up_en = false;

    // assign gpio number to 64bit memory (unsigned long long)
    // If we need multiple pins, we can OR them
    config.pin_bit_mask = ((1ULL<<PUSH_BUTTON_PIN) | (1ULL<<LED_PIN));

    // Set the gpio config
    gpio_config(&config);

    interputQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(buttonPushedTask, "buttonPushedTask", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(PUSH_BUTTON_PIN, gpio_isr_handler, (void *)PUSH_BUTTON_PIN);

}

// Digital to analog converter with 
void dac_demo()
{
    // Use channel 2 DAC (GPIO 26)
    dac_output_enable(DAC_CHANNEL_2);

    // Set the value of the pin
    // Since DAC is 8 bit, its val is 0-255
    dac_output_voltage(DAC_CHANNEL_2,150);

}

 // HACK
 // Use GPIO as 3.3V source (since there is only 1 3.3V pin on ESP32 board)
void set_dummy_voltage_source()
{
    gpio_pad_select_gpio(DUMMY_VCC_PIN);
    gpio_set_direction(DUMMY_VCC_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DUMMY_VCC_PIN, 1);
}

// Analog to digital converter
void adc_demo()
{

    // Configure the scale
    // 2^12 = 4095 => 1.1 V
    adc1_config_width(ADC_WIDTH_BIT_12);

    // Configure attenuation
    // Specify the ADC pin (GPIO39)
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_0);
    while (true)
    {
        // Read the analog value
        // Value will be in range 0-4095 (since 12 Bit is used)
        int val = adc1_get_raw(ADC1_CHANNEL_3);

        /*
            Readings
            Day (indoor): ~3500
            Day (cover with hand): ~1500
            Flash light on LDR: 4095 (max 12 bit value)
        */
        printf("value is %d\n", val);

        // wait for 1sec
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void pwd_demo()
{
    // Create timer structure
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT, // 0-1023
        .timer_num = LEDC_TIMER_0, // should be same in channel
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK};

    // Configure the timer
    ledc_timer_config(&timer);

    // Create the channel
    ledc_channel_config_t channel = {
        .gpio_num = LED_PIN, //GPIO 17
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0, // should match the one in timer
        .duty = 0, // Range depends on duty_resolution (here 0-1023)
        .hpoint = 0};

    // Configure the channel
    ledc_channel_config(&channel);

    // need for ledc_set_duty_and_update()
    ledc_fade_func_install(0);

    // Change the duty cycle
    // Range is 0-1023 since the duty_resolution is 10 bits
    for (int i = 0; i < 1024; i++)
    {
        // ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
        // ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        // Set and update the duty cycle
        ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i, 0);

        // S
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    // Alternate to to above logic
    while (true)
    {
        // Specify the target duty cycle value (0) and time required to reach to that value (1000 ms)
        // Turn off the LED slowly
        ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 1000, LEDC_FADE_WAIT_DONE);

        // Turn on the LED slowly
        ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1024, 1000, LEDC_FADE_WAIT_DONE);
    }
}

void app_main(void)
{

    // This is very simple onboard led blink
    //gpio_output_demo();

    // Create a queue for handling interrupt tasks
    interputQueue = xQueueCreate(10, sizeof(int));

    // Create a task that can run in background
    xTaskCreate(buttonPushedTask, "buttonPushedTask", 2048, NULL, 1, NULL);

    // This is the push button with interrupt (ISR)
    // gpio_input_demo();

    // multiple GPIO with config
    //gpio_config_demo();

    // Digital to analog converter
    // Usecase: control the brightness of LED
    //dac_demo();

    // Dummy voltage source
    set_dummy_voltage_source();

    // Analog to digital converter
    //adc_demo();

    // Pulse Width Modulation demo
    pwd_demo();

}

/*

voltage drop across LED = 2.75V
Max current that can flow through a GPIO = 0.02 A
Resistance required = 2.75/0.02 = 30 OHM

*/
