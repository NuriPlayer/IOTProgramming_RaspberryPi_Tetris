#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#ifdef USE_FPGA_VS
#include "fpga_io_vs.h"
#else
#include "fpga_io.h"
#endif

static int running = 1;
void handle_sigint(int sig) { running = 0; }

void clear_all() {
    int led[2][4] = {{0}};
    write_led(led);
    write_fnd((int[4]){0, 0, 0, 0});
    write_lcd("                ", "                ");
    write_dot((int[10][7]){0});
    write_buzzer(0);
}

// 음계 (0~7번 버튼용): 도레미파솔라시도
const int scale[][2] = {
    {262, 300}, {294, 300}, {330, 300}, {349, 300},
    {392, 300}, {440, 300}, {494, 300}, {523, 300}
};

// 간단한 테트리스 멜로디 일부
const int tetris[][2] = {
    {659, 100}, {494, 100}, {523, 100}, {587, 100}, {659, 100},
    {523, 100}, {494, 100}, {440, 100}, {440, 100}, {0, 100},
    {523, 100}, {659, 100}, {587, 100}, {523, 100}, {494, 100}
};

int main() {
    if (init_fpga_io() < 0) {
        fprintf(stderr, "FPGA IO 초기화 실패\n");
        return 1;
    }

    signal(SIGINT, handle_sigint);
    printf("FPGA 테스트 시작 (Ctrl+C로 종료)\n");

    int count = 0;

    while (running) {
        uint8_t push[9] = {0};
        if (read_push(push) > 0) {
            for (int i = 0; i < 9; ++i) {
                if (push[i]) {
                    count++;

                    int matrix[10][7] = {{0}};
                    int pos = (count-1) % 70;
                    matrix[pos / 7][pos % 7] = 1;
                    write_dot(matrix);

                    int led[2][4] = {{0}};
                    int idx = (count - 1) % 8;
                    led[idx / 4][idx % 4] = 1;
                    write_led(led);

                    int digits[4] = {
                        (count / 1000) % 10,
                        (count / 100) % 10,
                        (count / 10) % 10,
                        count % 10
                    };
                    write_fnd(digits);

                    char msg1[17], msg2[17];
                    snprintf(msg1, 17, "Count: %d", count);
                    snprintf(msg2, 17, "Button: %d", i);
                    write_lcd(msg1, msg2);

                    if (i >= 0 && i <= 7)
                        play_tone(scale[i][0], scale[i][1]);
                    else if (i == 8)
                        play_melody(tetris, sizeof(tetris) / sizeof(tetris[0]));

                    break;  // 여러 버튼 중 하나만 처리
                }
            }
        }
        usleep(150000);
    }

    clear_all();
    close_fpga_io();
    printf("\n테스트 종료.\n");
    return 0;
}
