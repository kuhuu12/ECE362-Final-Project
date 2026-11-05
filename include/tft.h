#ifndef TFT_H
#define TFT_H
#include <stdint.h>
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define YELLOW 0xFFE0
#define CYAN 0x07FF
void init_gpio(void);
void init_spi(void);
void init_tft(void);
void tft_fill_screen(uint16_t color);
void tft_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void tft_fill_rect(int x, int y, int w, int h, uint16_t color);
void draw_game(void);
void update_pipes(void);
void update_score(void);
void check_game_over(void);
#endif
