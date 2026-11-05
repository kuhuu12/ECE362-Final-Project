#include "pico/stdlib.h"
#include "hardware/spi.h"
#define TFT_DC   12
#define TFT_RST  13

#define TFT_SCK  10
#define TFT_MOSI 11
#define TFT_CS   9
#define BUTTON_PIN 21


#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// RGB565 colors
#define BLACK  0x0000
#define WHITE  0xFFFF
#define RED    0xF800
#define GREEN  0x07E0
#define BLUE   0x001F
#define YELLOW 0xFFE0

void init_gpio(void) {
    // Configure SPI pins
    gpio_set_function(TFT_SCK, GPIO_FUNC_SPI);
    gpio_set_function(TFT_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(TFT_CS, GPIO_FUNC_SPI);

    // Configure button
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
}

void init_spi(void) {
    // Initialize SPI1 at 10 MHz
    spi_init(spi1, 10000000);

    // Configure SPI format
    spi_set_format(spi1, 8, SPI_CPOL_0,
                   SPI_CPHA_0, SPI_MSB_FIRST);
}

//initalize tft control pins
void init_tft_gpio(void) {
    gpio_init(TFT_DC);
    gpio_set_dir(TFT_DC, GPIO_OUT);

    gpio_init(TFT_RST);
    gpio_set_dir(TFT_RST, GPIO_OUT);

    gpio_put(TFT_DC, 1);
    gpio_put(TFT_RST, 1);
}

//resets the display

void reset_tft(void) {
    gpio_put(TFT_RST, 0);
    sleep_ms(20);

    gpio_put(TFT_RST, 1);
    sleep_ms(120);
}

//sends commands to the display
gpio_init(TFT_CS);
gpio_set_dir(TFT_CS, GPIO_OUT);
gpio_put(TFT_CS, 1);

void tft_write_command(uint8_t cmd) {
    gpio_put(TFT_CS, 0);
    gpio_put(TFT_DC, 0);

    spi_write_blocking(spi1, &cmd, 1);

    gpio_put(TFT_CS, 1);
}

void tft_write_data(uint8_t data) {
    gpio_put(TFT_CS, 0);
    gpio_put(TFT_DC, 1);

    spi_write_blocking(spi1, &data, 1);

    gpio_put(TFT_CS, 1);
}

//initalize the tft 
void init_tft(void) {
    init_tft_gpio();
    reset_tft();

    // Software reset
    tft_write_command(0x01);
    sleep_ms(150);

    // Display OFF
    tft_write_command(0x28);

    // Pixel format: 16-bit RGB565
    tft_write_command(0x3A);
    tft_write_data(0x55);

    // Memory access control
    tft_write_command(0x36);
    tft_write_data(0x48);

    // Sleep OUT
    tft_write_command(0x11);
    sleep_ms(120);

    // Display ON
    tft_write_command(0x29);
    sleep_ms(20);
}

//selects where to draw
void tft_set_window(uint16_t x0, uint16_t y0,
                    uint16_t x1, uint16_t y1) {

    tft_write_command(0x2A);
    tft_write_data(x0 >> 8);
    tft_write_data(x0 & 0xFF);
    tft_write_data(x1 >> 8);
    tft_write_data(x1 & 0xFF);

    tft_write_command(0x2B);
    tft_write_data(y0 >> 8);
    tft_write_data(y0 & 0xFF);
    tft_write_data(y1 >> 8);
    tft_write_data(y1 & 0xFF);

    tft_write_command(0x2C);
}

//fills one pixel
void tft_draw_pixel(uint16_t x, uint16_t y,
                    uint16_t color) {

    if (x >= TFT_WIDTH || y >= TFT_HEIGHT) {
        return;
    }

    tft_set_window(x, y, x, y);

    tft_write_data(color >> 8);
    tft_write_data(color & 0xFF);
}

//writes function to fill entire screen
void tft_fill_screen(uint16_t color) {
    tft_set_window(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);

    uint8_t buffer[512];

    for (int i = 0; i < 512; i += 2) {
        buffer[i] = color >> 8;
        buffer[i + 1] = color & 0xFF;
    }

    gpio_put(TFT_CS, 0);
    gpio_put(TFT_DC, 1);

    for (int i = 0; i < (TFT_WIDTH * TFT_HEIGHT) / 256; i++) {
        spi_write_blocking(spi1, buffer, sizeof(buffer));
    }

    gpio_put(TFT_CS, 1);
}

//step 1-4 makes sure spi communicates with display;checks whether it shows color and pixels