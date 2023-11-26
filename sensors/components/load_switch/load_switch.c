#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "LOAD_MONITOR"

// Connected to MOSFET Gate
#define LOAD_GATE_GPIO 27

void load_switch(void *param)
{
    // Select the GPIO
    gpio_pad_select_gpio(LOAD_GATE_GPIO);

    // Define whether the Pin should be input or output
    gpio_set_direction(LOAD_GATE_GPIO, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "Controlling the MOSFET gate using GPIO: %d\n", LOAD_GATE_GPIO);

    int isOn = 0;
    while(1)
    {
        
        
        // Toggle the Pin
        isOn = !isOn;

        // Set the value of a GPIO pin
        gpio_set_level(LOAD_GATE_GPIO, isOn);
        ESP_LOGI(TAG, "Current Gate status: %d", isOn);
        if(isOn)
        {
            // P-Channel MOSFET - Source and Drain Disconnected
            // N-Channel MOSFET - Drain and Source Connected
            ESP_LOGI(TAG, "\nLoad is CONNECTED!! %d", isOn);
        }else
        {
            ESP_LOGI(TAG, "\nLoad is DIS-CONNECTED. %d", isOn);
        }

        // wait for 5 sec
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}