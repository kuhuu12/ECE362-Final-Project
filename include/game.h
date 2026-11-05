#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

extern bool game_over;

void game_run(void);
void reset_game(void);
void update_game_frame(void);
void show_game_over(void);

#endif