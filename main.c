#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <conio.h>
#include <stdint.h>
#include <sys/time.h>
#include "tetris_logic.h"
#ifdef USE_FPGA_VS
#include "fpga_io_vs.h"
#else
#include "fpga_io.h"
#endif

// 렌더링 함수는 tetris_render.c에서 제공
// void render_field(void);
// void print_next_block_preview(void);

unsigned long get_tick_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int main() {
    init_fpga_io();
    int buffer[FIELD_HEIGHT][FIELD_WIDTH];
    unsigned long last_fall_time = get_tick_ms();

    srand((unsigned int)time(NULL));
    init_field();
    generate_next_block();
    spawn_block();
    get_display_buffer(buffer);
    write_dot(buffer);

    // 첫 블록 스폰(실패 시 즉시 게임오버)
    if (!spawn_block()) {
        // printf("GAME OVER! SPAWN FAILED.\n");
        return 0;
    }

    while (1) {
        // ========== [1] 사용자 입력 처리 ==========
        int input_flag = 0;
        uint8_t push[9] = {0};
        if (read_push(push) > 0) {
            if (push[3]) {        // ←(왼쪽)
                try_move_block(-1);
                input_flag = 1;
            }
            if (push[5]) {        // →(오른쪽)
                try_move_block(1);
                input_flag = 1;
            }
            if (push[7]) {        // ↓(하드드롭)
                hard_drop();
                input_flag = 1;
            }
            if (push[1]) {        // ↑(회전)
                // 회전 함수        
                input_flag = 1;
            }
        }
    // 나머지 스위치도 개별적으로 처리

        // ========== [2] 1초마다 낙하 ==========
        unsigned long now = get_tick_ms();
        if (now - last_fall_time >= 1000) {
            last_fall_time = now;
            if (!falling_block_active) {
                if (!spawn_block()) {
                    printf("GAME OVER!\n");
                    break;
                }
            } else {
                move_block_down();
            }
            input_flag = 1;
        }

        // ========== [3] 디스플레이/렌더링 ==========
        if (input_flag) {
            // 1. 게임 상태를 buffer에 복사
            get_display_buffer(buffer);
            // 2. FPGA dot 매트릭스에 출력
            write_dot(buffer);

            // (옵션) 콘솔 출력도 원하면 추가
            // clear_screen();
            // render_field();
            // print_next_block_preview();
        }

        usleep(10000); // CPU 과점유 방지
    }

    
    close_fpga_io();
    return 0;
}