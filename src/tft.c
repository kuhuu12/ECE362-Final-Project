#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdlib.h>
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

float bird_y = 160.0f;
float bird_velocity = 0.0f;
int score = 0;
bool game_over = false;

#define JUMP_STRENGTH -5.0f
#define GRAVITY 0.25f
#define BIRD_SIZE 12
#define BIRD_X 60



#define PIPE_WIDTH 30
#define PIPE_GAP 90
#define PIPE_SPEED 2
#define PIPE_SPACING 130
#define NUM_PIPES 3


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

//step 1-4 makes sure spi communicates with display;checks whether it shows color and 


//step 5 Button input — make the bird jump
void check_button(void) {
    static bool last_button_state = false;

    // Button is active LOW
    bool button_pressed = (gpio_get(BUTTON_PIN) == 0);

    // Detect a new button press
    if (button_pressed && !last_button_state) {
        bird_velocity = JUMP_STRENGTH;
    }

    last_button_state = button_pressed;
}




//step 6Bird movement — add gravity and falling
void update_bird(void) {
    // Apply gravity
    bird_velocity += GRAVITY;

    // Update vertical position
    bird_y += bird_velocity;

    // Prevent bird from going above screen
    if (bird_y < 0) {
        bird_y = 0;
        bird_velocity = 0;
    }

    // Prevent bird from going below screen
    if (bird_y > TFT_HEIGHT - BIRD_SIZE) {
        bird_y = TFT_HEIGHT - BIRD_SIZE;
        bird_velocity = 0;
    }
}
void tft_fill_rect(int x, int y, int width,
                   int height, uint16_t color) {

    // Clip rectangle to screen boundaries
    if (width <= 0 || height <= 0) return;

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + width;
    int y1 = y + height;

    if (x1 > TFT_WIDTH) x1 = TFT_WIDTH;
    if (y1 > TFT_HEIGHT) y1 = TFT_HEIGHT;

    if (x0 >= x1 || y0 >= y1) return;

    tft_set_window(x0, y0, x1 - 1, y1 - 1);

    uint8_t buffer[512];

    for (int i = 0; i < 512; i += 2) {
        buffer[i] = color >> 8;
        buffer[i + 1] = color & 0xFF;
    }

    int pixels = (x1 - x0) * (y1 - y0);

    gpio_put(TFT_CS, 0);
    gpio_put(TFT_DC, 1);

    while (pixels > 0) {
        int count = pixels > 256 ? 256 : pixels;

        spi_write_blocking(spi1, buffer, count * 2);

        pixels -= count;
    }

    gpio_put(TFT_CS, 1);
}

void draw_bird(void) {
    tft_fill_rect(BIRD_X, (int)bird_y,
                  BIRD_SIZE, BIRD_SIZE, YELLOW);
}

void erase_bird(int previous_y) {
    tft_fill_rect(BIRD_X, previous_y,
                  BIRD_SIZE, BIRD_SIZE, BLUE);
}

void update_game_frame(void) {
    if (game_over) {
        return;
    }

    // Check button input
    check_button();

    // Update bird movement
    update_bird();

    // Move pipes
    update_pipes();

    // Detect collisions
    check_game_over();

    // Update score only if still playing
    if (!game_over) {
        update_score();
    }

    // Draw updated game
    draw_game();
}
//step 7 pipes — generate obstacles and move them across the screen

typedef struct {
    int x;
    int gap_y;
    bool scored;
} Pipe;

Pipe pipes[NUM_PIPES];
void init_pipes(void) {
    for (int i = 0; i < NUM_PIPES; i++) {
        pipes[i].x = TFT_WIDTH + i * PIPE_SPACING;

        pipes[i].gap_y = 40 + rand() %
            (TFT_HEIGHT - PIPE_GAP - 80);

        pipes[i].scored = false;
    }
}

void draw_pipe(Pipe *pipe) {
    // Top pipe
    tft_fill_rect(pipe->x, 0,
                  PIPE_WIDTH, pipe->gap_y, GREEN);

    // Bottom pipe
    int bottom_y = pipe->gap_y + PIPE_GAP;

    tft_fill_rect(pipe->x, bottom_y,
                  PIPE_WIDTH, TFT_HEIGHT - bottom_y,
                  GREEN);
}

void update_pipes(void) {
    for (int i = 0; i < NUM_PIPES; i++) {
        pipes[i].x -= PIPE_SPEED;

        // Pipe has moved off the left side
        if (pipes[i].x + PIPE_WIDTH < 0) {

            // Find the rightmost pipe
            int rightmost_x = TFT_WIDTH;

            for (int j = 0; j < NUM_PIPES; j++) {
                if (pipes[j].x > rightmost_x) {
                    rightmost_x = pipes[j].x;
                }
            }

            // Reposition pipe to the right
            pipes[i].x = rightmost_x + PIPE_SPACING;

            // Generate a new opening
            pipes[i].gap_y = 40 + rand() %
                (TFT_HEIGHT - PIPE_GAP - 80);

            pipes[i].scored = false;
        }
    }
}
void erase_pipe(Pipe *pipe) {
    tft_fill_rect(pipe->x, 0,
                  PIPE_WIDTH, pipe->gap_y, BLUE);

    int bottom_y = pipe->gap_y + PIPE_GAP;

    tft_fill_rect(pipe->x, bottom_y,
                  PIPE_WIDTH, TFT_HEIGHT - bottom_y,
                  BLUE);
}
void update_pipe_frames(void) {
    // Erase old pipe positions
    for (int i = 0; i < NUM_PIPES; i++) {
        erase_pipe(&pipes[i]);
    }

    // Move pipes
    update_pipes();

    // Draw new pipe positions
    for (int i = 0; i < NUM_PIPES; i++) {
        draw_pipe(&pipes[i]);
    }
}




//step 8-Collision detection and scoring — detect crashes and track points

bool check_pipe_collision(void) {
    for (int i = 0; i < NUM_PIPES; i++) {

        // Check horizontal overlap
        bool x_collision =
            (BIRD_X < pipes[i].x + PIPE_WIDTH) &&
            (BIRD_X + BIRD_SIZE > pipes[i].x);

        // Check vertical overlap
        bool y_collision =
            (bird_y < pipes[i].gap_y) ||
            (bird_y + BIRD_SIZE >
             pipes[i].gap_y + PIPE_GAP);

        if (x_collision && y_collision) {
            return true;
        }
    }

    return false;
}

bool check_boundary_collision(void) {
    if (bird_y <= 0 ||
        bird_y >= TFT_HEIGHT - BIRD_SIZE) {
        return true;
    }

    return false;
}

void update_score(void) {
    for (int i = 0; i < NUM_PIPES; i++) {

        // Bird has passed the pipe
        if (!pipes[i].scored &&
            pipes[i].x + PIPE_WIDTH < BIRD_X) {

            score++;
            pipes[i].scored = true;
        }
    }
}
void check_game_over(void) {
    if (check_pipe_collision() ||
        check_boundary_collision()) {

        game_over = true;
    }
}
void reset_game(void) {
    // Reset bird
    bird_y = 160.0f;
    bird_velocity = 0.0f;

    // Reset score
    score = 0;

    // Reset pipes
    init_pipes();

    // Resume game
    game_over = false;
}

//step 9 combines
void draw_game(void) {
    // Clear background
    tft_fill_screen(BLUE);

    // Draw pipes
    for (int i = 0; i < NUM_PIPES; i++) {
        draw_pipe(&pipes[i]);
    }

    // Draw bird last
    draw_bird();
}

void show_game_over(void) {
    tft_fill_screen(RED);
}
bool restart_button_pressed(void) {
    static bool last_state = false;

    bool current_state = (gpio_get(BUTTON_PIN) == 0);

    bool new_press = current_state && !last_state;

    last_state = current_state;

    return new_press;
}
