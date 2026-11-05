//main file
#include "tft.h"
#include "input.h"
#include "sound.h"
#include "leds.h"
#include "game.h"

int main(void) {
    stdio_init_all();

    init_gpio();
    init_spi();
    init_tft();

    // Test the display
    tft_fill_screen(BLUE);
    sleep_ms(1000);

    tft_fill_screen(GREEN);
    sleep_ms(1000);

    tft_fill_screen(BLACK);

    // Draw one yellow pixel
    tft_draw_pixel(120, 160, YELLOW);

    while (true) {
        tight_loop_contents();
    }
}