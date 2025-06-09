#ifndef __FPGA_IO_H__
#define __FPGA_IO_H__

int init_fpga_io();
void close_fpga_io();

// LED
int write_led(const int led[2][4]);

// FND
int write_fnd(const int digits[4]);

// DOT MATRIX
int write_dot(const int matrix[10][7]);

// LCD
int write_lcd(const char *line1, const char *line2);

// BUZZER
int write_buzzer(int value);
int play_tone(int freq_hz, int duration_ms);
int play_melody(const int notes[][2], int length);

// PUSH SWITCH
int read_push(uint8_t push[9]);

#endif
