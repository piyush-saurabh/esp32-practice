#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ADC
#include <driver/adc.h>
#include <esp_adc_cal.h>

// ADC1_CHANNEL_7 is GPIO 35
// Ref: https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/adc.html
#define BATTERY_MONITOR_GPIO ADC1_CHANNEL_7

#define TAG "BATTERY_MONITOR"

void battery_monitor(void *param)
{
    ESP_LOGI(TAG, "Measuring the battery voltage ON GPIO: %d\n", BATTERY_MONITOR_GPIO);

    // Measure voltage in mV
    uint32_t voltage;

    // Caliberate ADC
    // Ref: https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/adc.html
    // Channel 1: ADC_UNIT_1
    // ADC attenuation: ADC_ATTEN_DB_11
    // ADC Width Bit: 12 bits by default
    ESP_LOGI(TAG, "Caliberating ADC\n");
    esp_adc_cal_characteristics_t adc1_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(BATTERY_MONITOR_GPIO, ADC_ATTEN_DB_11));

    

    while(1)
    {
        // Capture raw data
        // GPIO 35
        int raw_value = adc1_get_raw(BATTERY_MONITOR_GPIO);

        // Convert raw value to mV after calibration
        voltage = esp_adc_cal_raw_to_voltage(raw_value, &adc1_chars);

        ESP_LOGI(TAG, "Raw: %d, Voltage: %d mV", raw_value, voltage);

        // wait for 2sec
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}