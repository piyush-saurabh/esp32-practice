# OLED Display

## OLED Description
- Driver IC: SSD1306
- 0.96"
- 128x64 OLED display (width: 128, height: 64) 
- Supports I2C and SPI
- https://robu.in/product/0-96-oled-display-module-spii2c-128x64-7-pin-blue/

## Pin Connection
SPI Interface

OLED|Description|ESP32
----|-----------|-----
D0|Serial Clock / SCK | GPIO 22
D1|Serial Data / MOSI | GPIO 23
CD|Data/Command / MISO| GPIO 19
CS|Chip Select | GPIO 5
RES|Reset | GPIO 15
VCC|3.3 V | 3V3
GND|Ground|GND

## Display

- Top 2 lines are yellow, rest are blue
- `ssd1306_clear_screen(&dev, false)`: Clear the screen using . Second parameter is `invert`. **false** will result in black background with yellow (top)/blue(bottom) text.
- `ssd1306_contrast(&dev, 0xff)`: Set the contrast of the screen

### Display Text
- **ssd1306_display_text_x3**: Display large text. 1 text covers 3 lines, 5 characters
- **ssd1306_display_text**: Display small text. 8 lines, 16 characters

### Display Image
- `ssd1306_display_image(&dev, x, y, uint8_t img[24], sizeof(img))`

### Display Image




Reference
- https://esp32tutorials.com/oled-esp32-esp-idf-tutorial/#google_vignette
- https://github.com/nopnop2002/esp-idf-ssd1306/
