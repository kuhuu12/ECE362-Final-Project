//main file
#include "tft.h"
#include "input.h"
#include "sound.h"
#include "leds.h"
#include "game.h"

int main(void) {
    tft_init();          // start each part once
    input_init();
    sound_init();
    leds_init();
    game_init();

    while (1) {          // then loop forever
        bool flap = input_flap();
        uint32_t ev = game_update(flap);
        if (ev & EV_CRASHED) { sound_play(SND_GAMEOVER); leds_fill(40, 0, 0); }
        sound_update();
        sleep_ms(20);
    }
}