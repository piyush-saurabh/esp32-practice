#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ADC
#include <driver/adc.h>
#include <esp_adc_cal.h>

// ADC1_CHANNEL_6 is GPIO 34
#define LIGHT_SENSOR_GPIO ADC1_CHANNEL_6

#define TAG "LIGHT_SENSOR"

void light_sensor(void *param)
{
    ESP_LOGI(TAG, "Measuring the voltage divider drop voltage ON GPIO: %d\n", LIGHT_SENSOR_GPIO);

    // Caliberate ADC
    // Ref: https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/adc.html
    ESP_LOGI(TAG, "Caliberating ADC\n");
    esp_adc_cal_characteristics_t adc1_chars;

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc1_chars);

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(LIGHT_SENSOR_GPIO, ADC_ATTEN_DB_11));

    
    while(1)
    {
        uint32_t raw_value =  adc1_get_raw(LIGHT_SENSOR_GPIO);

        // Measure voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_value, &adc1_chars);

        ESP_LOGI(TAG, "Raw: %d, Voltage: %d mV", raw_value, voltage);

        // wait for 2sec
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}