#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "ssd1306.h"
#include "font8x8_basic.h"

#define TAG "MAIN"

// Comment it to stop debugging
#define DEBUG

void app_main()
{
#ifdef DEBUG
    ESP_LOGI(TAG, "Started...");
#endif

    SSD1306_t dev;
    int center, top, bottom, horizontal, vertical;
    char lineChar[20];

    ESP_LOGI(TAG, "INTERFACE is SPI");
    ESP_LOGI(TAG, "CONFIG_MOSI_GPIO=%d", CONFIG_MOSI_GPIO);
    ESP_LOGI(TAG, "CONFIG_SCLK_GPIO=%d", CONFIG_SCLK_GPIO);
    ESP_LOGI(TAG, "CONFIG_CS_GPIO=%d", CONFIG_CS_GPIO);
    ESP_LOGI(TAG, "CONFIG_DC_GPIO=%d", CONFIG_DC_GPIO);
    ESP_LOGI(TAG, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);

#if CONFIG_FLIP
    dev._flip = true;
    ESP_LOGW(TAG, "Flip upside down");
#endif

    // Initiate the connection over I2C interafce
    // i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

    // Initialize the SPI
    spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);

    // Display is 128x64 (0.96")
    ESP_LOGI(TAG, "Initializing the display");
    ssd1306_init(&dev, 128, 64);

    // Clear the screen
    // Background will be dark and text will be white
    ssd1306_clear_screen(&dev, true);

    // Set contrast to high
    ssd1306_contrast(&dev, 0xff);

    // Display Image
    vertical = 3;
    horizontal = 5;
    // uint8_t img[24];

    // Bitmap generated from: https://javl.github.io/image2cpp/
    uint8_t img[] = {0xcf, 0xc7, 0xe3, 0x73, 0x31, 0x19, 0x98, 0x88, 0xcc, 0xcc, 0xcc, 0xcc, 0x88, 0x98, 0x19, 0x31, 
0x73, 0xe3, 0xc7, 0xcf, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xff, 0xf9, 0xf8, 0xfc, 0x8c, 0x8c, 0xfc, 
0xf8, 0xf9, 0xff, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};

    ssd1306_display_image(&dev, vertical, horizontal, img, sizeof(img));
    vTaskDelay(15000 / portTICK_PERIOD_MS);

    ssd1306_clear_screen(&dev, false);

    ssd1306_display_text(&dev, 0, "MENU", 4, true);
    ssd1306_display_text(&dev, 1, "sub menu", 8, false);

    // Display the text
    ssd1306_display_text_x3(&dev, 3, "Hi !!", 5, false);

    vTaskDelay(8000 / portTICK_PERIOD_MS);

    // Line number on teh screen
    top = 2;
    center = 3;
    bottom = 8;

    // Black background
    ssd1306_display_text(&dev, 0, "SSD1306 128x64", 14, false);
    ssd1306_display_text(&dev, 1, "ABCDEFGHIJKLMNOP", 16, false);
    ssd1306_display_text(&dev, 2, "abcdefghijklmnop", 16, false);
    ssd1306_display_text(&dev, 3, "Hello World!!", 13, false);

    // Colored (yellow/blue) background
    ssd1306_display_text(&dev, 4, "SSD1306 128x64", 14, true);
    ssd1306_display_text(&dev, 5, "ABCDEFGHIJKLMNOP", 16, true);
    ssd1306_display_text(&dev, 6, "abcdefghijklmnop", 16, true);
    ssd1306_display_text(&dev, 7, "Hello World!!", 13, true);

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Display Numbers
    uint8_t image[24];
    memset(image, 0, sizeof(image));
    ssd1306_display_image(&dev, top, (6 * 8 - 1), image, sizeof(image));
    ssd1306_display_image(&dev, top + 1, (6 * 8 - 1), image, sizeof(image));
    ssd1306_display_image(&dev, top + 2, (6 * 8 - 1), image, sizeof(image));
    for (int font = 0x39; font > 0x30; font--)
    {
        memset(image, 0, sizeof(image));
        ssd1306_display_image(&dev, top + 1, (7 * 8 - 1), image, 8);
        memcpy(image, font8x8_basic_tr[font], 8);
        if (dev._flip)
            ssd1306_flip(image, 8);
        ssd1306_display_image(&dev, top + 1, (7 * 8 - 1), image, 8);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Scroll Up
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, 0, "---Scroll  UP---", 16, true);

    ssd1306_software_scroll(&dev, (dev._pages - 1), 1);
    for (int line = 0; line < bottom + 10; line++)
    {
        lineChar[0] = 0x01;
        sprintf(&lineChar[1], " Line %02d", line);
        ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Scroll Down
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, 0, "--Scroll  DOWN--", 16, true);
    // ssd1306_software_scroll(&dev, 1, 7);
    ssd1306_software_scroll(&dev, 1, (dev._pages - 1));
    for (int line = 0; line < bottom + 10; line++)
    {
        lineChar[0] = 0x02;
        sprintf(&lineChar[1], " Line %02d", line);
        ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Page Down
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, 0, "---Page	DOWN---", 16, true);
    ssd1306_software_scroll(&dev, 1, (dev._pages - 1));
    for (int line = 0; line < bottom + 10; line++)
    {

        if ((line % (dev._pages - 1)) == 0)
            ssd1306_scroll_clear(&dev);
        lineChar[0] = 0x02;
        sprintf(&lineChar[1], " Line %02d", line);
        ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Horizontal Scroll
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, center, "Horizontal", 10, false);
    ssd1306_hardware_scroll(&dev, SCROLL_RIGHT);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ssd1306_hardware_scroll(&dev, SCROLL_LEFT);
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // Vertical Scroll
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, center, "Vertical", 8, false);
    ssd1306_hardware_scroll(&dev, SCROLL_DOWN);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ssd1306_hardware_scroll(&dev, SCROLL_UP);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ssd1306_hardware_scroll(&dev, SCROLL_STOP);

    // Invert
    ssd1306_clear_screen(&dev, true);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, center, "  Good Bye!!", 12, true);
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    ssd1306_fadeout(&dev);
    esp_restart();
}