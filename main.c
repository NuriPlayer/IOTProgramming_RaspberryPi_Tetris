#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
// #include <conio.h>
#include <stdint.h>
#include <sys/time.h>
#include "tetris_logic.h"
#ifdef USE_FPGA_VS
#include "fpga_io_vs.h"
#else
#include "fpga_io.h"
#endif

unsigned long get_tick_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void clear_fpga_device() {
    int led[2][4] = {{0}};
    write_led(led);
    write_fnd((int[4]){0, 0, 0, 0});
    write_lcd("                ", "                ");
    write_dot((int[10][7]){0});
    write_buzzer(0);
}

int wait_button_and_prompt_restart() {
    unsigned long start = get_tick_ms();
    while (1) {
        uint8_t push[9] = {0};
        if (read_push(push) > 0) {
            int pressed = 0;
            for (int i = 0; i < 9; ++i)
                if (push[i]) pressed = 1;
            if (pressed) return 1;
        }
        if (get_tick_ms() - start > 10000) break;
        usleep(10000);
    }
    return 0;
}

void show_score_on_fnd(int score) {
    int digits[4] = {
        (score / 1000) % 10,
        (score / 100) % 10,
        (score / 10) % 10,
        score % 10
    };
    write_fnd(digits);
}

void show_next_block_on_led(void) {
    int led[2][4] = {{0}};
    const int is4 = next_block_type >= 6;
    const int type = is4 ? next_block_type - 6 : next_block_type;
    const int size = is4 ? 4 : 3;

    int offset = 0;
    if (is4) {
        for (; offset < 4; ++offset) {
            int any = 0;
            for (int x = 0; x < 4; ++x)
                if (tetro4x4[type][offset][x] == 2) { any = 1; break; }
            if (any) break;
        }
        if (offset > size - 2) offset = size - 2;
    } else {
        for (; offset < 3; ++offset) {
            int any = 0;
            for (int x = 0; x < 3; ++x)
                if (tetro3x3[type][offset][x] == 2) { any = 1; break; }
            if (any) break;
        }
        if (offset > size - 2) offset = size - 2;
    }

    for (int y = 0; y < 2; ++y) {
        int ty = y + offset;
        for (int x = 0; x < size; ++x) {
            int val = is4
                ? tetro4x4[type][ty][x]
                : tetro3x3[type][ty][x];
            led[y][x] = (val == 2) ? 1 : 0;
        }
        for (int x = size; x < 4; ++x)
            led[y][x] = 0;
    }

    write_led(led);
}


void play_game_over_melody() {
    play_tone(262, 300); // 도
    usleep(100000);
    play_tone(196, 300); // 솔
    usleep(100000);
    play_tone(130, 600); // 낮은 도
}


void blink_dot_matrix(int buffer[FIELD_HEIGHT][FIELD_WIDTH], int times, int interval_ms) {
    for (int i = 0; i < times; ++i) {
        int blank[10][7] = {0};
        write_dot(blank);
        usleep(interval_ms * 1000);

        write_dot(buffer);
        usleep(interval_ms * 1000);
    }
}

int main() {
    init_fpga_io();
    clear_fpga_device();

    const int MAX_DIFFICULTY = 4;
    const int fall_delay_table[5] = {1000, 900, 800, 650, 500};

    while (1) {
        int buffer[FIELD_HEIGHT][FIELD_WIDTH];
        unsigned long last_fall_time = get_tick_ms();
        unsigned long game_start_time = get_tick_ms();
        unsigned long last_difficulty_update = game_start_time;
        srand((unsigned int)time(NULL));
        init_field();
        int game_over = 0;
        difficulty = 0;
        generate_next_block();
        if (!spawn_block()) {
            game_over = 1;
            break;
        }
        get_display_buffer(buffer);
        write_dot(buffer);

        char line2[17];
        unsigned long now = get_tick_ms();
        unsigned long elapsed_ms = now - game_start_time;
        int minute = (elapsed_ms / 1000) / 60;
        int second = (elapsed_ms / 1000) % 60;
        write_lcd("  TETRIS GAME!   ", "Play Time: 0:00");
        score = 0;
        show_score_on_fnd(0);
        show_next_block_on_led();

        while (1) {
            int input_flag = 0;
            uint8_t push[9] = {0};
            if (read_push(push) > 0) {
                if (push[3]) { try_move_block(-1); input_flag = 1; play_tone(392, 300); }
                if (push[5]) { try_move_block(1); input_flag = 1; play_tone(392, 300); }
                if (push[7]) { hard_drop(); input_flag = 1; }
                if (push[1]) { rotate_falling_block(); input_flag = 1; play_tone(392, 300); }
            }

            unsigned long now = get_tick_ms();

            if (now - last_difficulty_update >= 60000 && difficulty < MAX_DIFFICULTY) {
                difficulty += 1;
                last_difficulty_update += 60000;
            }

            int fall_delay = fall_delay_table[difficulty];

            if (now - last_fall_time >= fall_delay) {
                last_fall_time = now;
                if (!falling_block_active) {
                    if (!spawn_block()) {
                        write_lcd(" - GAME OVER! - ", "PUSH TO RESTART.");

                        
                        get_display_buffer(buffer);

                        
                        play_game_over_melody();

                        
                        blink_dot_matrix(buffer, 3, 300);

                        break;
                    }
                } else {
                    move_block_down();
                }
                input_flag = 1;
            }

            int lines = clear_full_lines();
            if (lines > 0) {
                add_score(lines);
            }
            if (block_fixed_flag) {
                play_tone(392, 300);
                play_tone(523, 300);
                block_fixed_flag = 0;
                input_flag = 1;
            }

            if (input_flag) {
                get_display_buffer(buffer);
                write_dot(buffer);

                unsigned long elapsed_ms = now - game_start_time;
                int minute = (elapsed_ms / 1000) / 60;
                int second = (elapsed_ms / 1000) % 60;
                snprintf(line2, sizeof(line2), "Play Time:%2d:%02d", minute, second);
                write_lcd("  TETRIS GAME!   ", line2);

                show_score_on_fnd(score);
                show_next_block_on_led();
            }
            usleep(10000);
        }

        if (!wait_button_and_prompt_restart()) break;
    }
    close_fpga_io();
    return 0;
}
