//main file
#include "tft.h"
#include "input.h"
#include "sound.h"
#include "leds.h"
#include "game.h"

int main(void) {
    // Initialize standard I/O
    stdio_init_all();

    // Step 1: Initialize GPIO
    init_gpio();

    // Step 2: Initialize SPI1
    init_spi();

    // Step 3: Initialize TFT display
    init_tft();

    // Initialize game variables and pipes
    reset_game();

    // Draw initial screen
    draw_game();

    // Track whether the game-over screen was drawn
    bool game_over_drawn = false;

    // Main game loop
    while (true) {

        if (!game_over) {

            // Update bird, pipes, collisions and screen
            update_game_frame();

            game_over_drawn = false;

        } else {

            // Show game-over screen once
            if (!game_over_drawn) {
                show_game_over();
                game_over_drawn = true;
            }

            // Wait for button release
            while (gpio_get(BUTTON_PIN) == 0) {
                sleep_ms(10);
            }

            // Wait for a new button press
            while (gpio_get(BUTTON_PIN) != 0) {
                sleep_ms(10);
            }

            // Debounce
            sleep_ms(20);

            if (gpio_get(BUTTON_PIN) == 0) {
                reset_game();
                draw_game();
            }

            // Wait until button is released
            while (gpio_get(BUTTON_PIN) == 0) {
                sleep_ms(10);
            }
        }

        // Target approximately 50 FPS
        sleep_ms(20);
    }
}