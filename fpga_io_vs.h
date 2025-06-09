#ifndef FPGA_IO_VS_H
#define FPGA_IO_VS_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>   // _kbhit, _getch
#include <windows.h> // Beep, Sleep

int init_fpga_io() {
    printf("[VS] FPGA IO initialized (Windows virtual mode).\n");
    return 0;
}

void close_fpga_io() {
    printf("[VS] FPGA IO closed (Windows virtual mode).\n");
}

int write_dot(int matrix[10][7]) {
    system("cls");
    printf("[VS] Dot Matrix Display:\n");
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 7; ++j)
            printf("%c", matrix[i][j] ? '*' : '.');
        printf("\n");
    }
    return 0;
}

int write_fnd(int digits[4]) {
    printf("[VS] FND: %d%d%d%d\n", digits[0], digits[1], digits[2], digits[3]);
    return 0;
}

int write_lcd(const char* line1, const char* line2) {
    printf("[VS] LCD:\n");
    printf("|%-16s|\n", line1);
    printf("|%-16s|\n", line2);
    return 0;
}

int write_led(int led[2][4]) {
    printf("[VS] LED:\n");
    for (int i = 0; i < 2; ++i) {
        printf("Row %d: ", i);
        for (int j = 0; j < 4; ++j)
            printf("%d", led[i][j]);
        printf("\n");
    }
    return 0;
}

int write_buzzer(int on) {
    printf("[VS] Buzzer: %s\n", on ? "ON" : "OFF");
    return 0;
}

int beep_buzzer(int freq_hz, int duration_ms) {
    printf("[VS] Beep: %d Hz for %d ms\n", freq_hz, duration_ms);
    Beep(freq_hz, duration_ms);  // Windows 기본 사운드 출력
    return 0;
}

int write_motor(int dir, int speed) {
    printf("[VS] Motor: Direction=%d, Speed=%d\n", dir, speed);
    return 0;
}

// 키보드 입력 → 푸시 스위치 배열 (숫자 '1'~'9' 키)
int read_push(uint8_t push[9]) {
    memset(push, 0, 9);
    if (_kbhit()) {
        int key = _getch();
        if (key >= '1' && key <= '9') {
            // 물리 배열에 대응되도록 매핑
            int map[9] = {6, 7, 8, 3, 4, 5, 0, 1, 2}; // 숫자키 1~9 -> 3x3 위치
            int index = map[key - '1'];
            push[index] = 1;
            return 1;
        }
    }
    return 0;
}

int play_tone(int freq_hz, int duration_ms) {
    printf("[VS] Tone: %d Hz for %d ms\n", freq_hz, duration_ms);
    if (freq_hz > 0 && duration_ms > 0)
        Beep(freq_hz, duration_ms);
    else
        Sleep(duration_ms);
    return 0;
}

int play_melody(const int notes[][2], int length) {
    for (int i = 0; i < length; ++i) {
        if (notes[i][0] == 0)
            Sleep(notes[i][1]);
        else
            play_tone(notes[i][0], notes[i][1]);
        Sleep(20); // 간격
    }
    return 0;
}

#endif // FPGA_IO_VS_H

