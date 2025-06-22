#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "fpga_io.h"

// 디바이스 파일 경로
#define DEV_LED "/dev/fpga_led"
#define DEV_FND "/dev/fpga_fnd"
#define DEV_DOT "/dev/fpga_dot"
#define DEV_LCD "/dev/fpga_text_lcd"
#define DEV_BUZZER "/dev/fpga_buzzer"
#define DEV_PUSH "/dev/fpga_push_switch"

int fd_led = -1, fd_fnd = -1, fd_dot = -1, fd_lcd = -1, fd_buzzer = -1, fd_push = -1;

int init_fpga_io() {
    fd_led = open(DEV_LED, O_RDWR);
    fd_fnd = open(DEV_FND, O_RDWR);
    fd_dot = open(DEV_DOT, O_RDWR);
    fd_lcd = open(DEV_LCD, O_RDWR);
    fd_buzzer = open(DEV_BUZZER, O_RDWR);
    fd_push = open(DEV_PUSH, O_RDONLY);

    return (fd_led < 0 || fd_fnd < 0 || fd_dot < 0 || fd_lcd < 0 || fd_buzzer < 0 || fd_push < 0) ? -1 : 0;
}

void close_fpga_io() {
    if (fd_led >= 0) close(fd_led);
    if (fd_fnd >= 0) close(fd_fnd);
    if (fd_dot >= 0) close(fd_dot);
    if (fd_lcd >= 0) close(fd_lcd);
    if (fd_buzzer >= 0) close(fd_buzzer);
    if (fd_push >= 0) close(fd_push);
}

int write_led(const int led[2][4]) {
    uint8_t val = 0;
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 4; ++j)
            val |= (led[1 - i][3 - j] ? 1 : 0) << (i * 4 + j);

    return write(fd_led, &val, 1);
}

int write_fnd(const int digits[4]) {
    uint8_t data[4];
    for (int i = 0; i < 4; ++i) data[i] = digits[i];
    return write(fd_fnd, data, 4);
}

int write_dot(const int matrix[10][7]) {
    uint8_t data[10] = {0};
    for (int row = 0; row < 10; ++row)
        for (int col = 0; col < 7; ++col)
            data[row] |= (matrix[row][col] ? 1 : 0) << (6 - col);

    return write(fd_dot, data, 10);
}

int write_lcd(const char *line1, const char *line2) {
    char data[33] = {0};
    snprintf(data, 17, "%-16s", line1);
    snprintf(data + 16, 17, "%-16s", line2);
    return write(fd_lcd, data, 32);
}

int write_buzzer(int value) {
    uint8_t v = (value ? 1 : 0);
    return write(fd_buzzer, &v, 1);
}

// 내부 지연 함수
static void delay_us(int us) {
    struct timespec ts = {.tv_sec = us / 1000000, .tv_nsec = (us % 1000000) * 1000};
    nanosleep(&ts, NULL);
}

int play_tone(int freq_hz, int duration_ms) {
    if (fd_buzzer < 0 || freq_hz <= 0 || duration_ms <= 0) return -1;

    int period = 1000000 / freq_hz;
    int on = period * 9 / 10;
    int off = period / 10;
    int cycles = (duration_ms * 1000) / period;

    for (int i = 0; i < cycles; ++i) {
        write_buzzer(1);
        delay_us(on);
        write_buzzer(0);
        delay_us(off);
    }
    return 0;
}

int play_melody(const int notes[][2], int length) {
    for (int i = 0; i < length; ++i) {
        if (notes[i][0] == 0)
            usleep(notes[i][1] * 1000);
        else
            play_tone(notes[i][0], notes[i][1]);
        usleep(20000);
    }
    return 0;
}

// 디바운싱 시간(ms)
#define DEBOUNCE_DELAY_MS 200

// 마지막으로 버튼을 눌렀던 시간 저장용
static struct timeval last_press_time = {0, 0};

int read_push(uint8_t push[9]) {
    uint8_t buffer[9];
    int result = read(fd_push, buffer, 9);
    if (result < 0) return result;

    // 현재 시간 가져오기
    struct timeval now;
    gettimeofday(&now, NULL);

    // 시간 차이 계산
    long elapsed_ms = (now.tv_sec - last_press_time.tv_sec) * 1000 +
                      (now.tv_usec - last_press_time.tv_usec) / 1000;

    // 눌린 버튼 확인
    int pressed = 0;
    for (int i = 0; i < 9; i++) {
        if (buffer[i]) {
            pressed = 1;
            break;
        }
    }

    if (pressed) {
        // 마지막 버튼 누름 이후 디바운스 시간 경과했는지 확인
        if (elapsed_ms >= DEBOUNCE_DELAY_MS) {
            memcpy(push, buffer, 9);  // 유효 입력 전달
            gettimeofday(&last_press_time, NULL);  // 마지막 누름 시간 갱신
            return result;  // 버튼 입력 정상 리턴
        } else {
            // 무시할 입력 (디바운스 처리)
            memset(push, 0, 9);
            return 0;  // 읽긴 했지만 무시
        }
    }

    // 버튼 안 눌렸으면 그냥 리턴
    memset(push, 0, 9);
    return result;
}



