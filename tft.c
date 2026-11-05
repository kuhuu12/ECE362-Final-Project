#include "pico/stdlib.h"
#include "hardware/spi.h"

#define TFT_SCK  10
#define TFT_MOSI 11
#define TFT_CS   9
#define BUTTON_PIN 21

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