#include "tetris_logic.h"
#include <stdio.h>
#include <stdlib.h>

// 게임판
int field[FIELD_HEIGHT][FIELD_WIDTH];
int next_block_type;

int falling_block_x;
int falling_block_y;
int falling_block_type;
int falling_block_active;

// 3블록 테트로미노 (회전 포함, 3x3)
int tetro3x3[6][3][3] = {
    {{0,0,0},{2,2,2},{0,0,0}}, // 가로 일자
    {{0,2,0},{0,2,0},{0,2,0}}, // 세로 일자
    {{0,2,0},{0,2,2},{0,0,0}}, // ㄴ (왼쪽 아래)
    {{0,2,0},{2,2,0},{0,0,0}}, // ㄴ (오른쪽 아래)
    {{0,0,0},{0,2,2},{0,2,0}}, // ㄴ (왼쪽 위)
    {{0,0,0},{2,2,0},{0,2,0}}, // ㄴ (오른쪽 위)
};

static int get_top_offset(int type) {
    int offset = 0;
    for (int y = 0; y < 3; y++) {
        int empty = 1;
        for (int x = 0; x < 3; x++)
            if (tetro3x3[type][y][x] == 2) empty = 0;
        if (empty) offset++;
        else break;
    }
    return offset;
}

void init_field(void) {
    for (int y = 0; y < FIELD_HEIGHT; y++)
        for (int x = 0; x < FIELD_WIDTH; x++)
            field[y][x] = 0;
}

void generate_next_block(void) {
    next_block_type = rand() % 6;
}

int spawn_block(void) {
    // 1. 게임 종료 체크
    if (is_game_over()) {
        return 0; // 스폰 실패 = 게임오버
    }
    // 2. 실제 블록 스폰
    falling_block_type = next_block_type;
    int offset = get_top_offset(falling_block_type);
    falling_block_x = 2;
    falling_block_y = -offset;
    falling_block_active = 1;
    generate_next_block();
    return 1; // 스폰 성공
}

// 충돌판정/자동 고정
void move_block_down(void) {
    int can_move = 1;
    for (int by = 0; by < 3; by++) {
        for (int bx = 0; bx < 3; bx++) {
            if (tetro3x3[falling_block_type][by][bx] == 2) {
                int nx = falling_block_x + bx;
                int ny = falling_block_y + by + 1;
                if (ny >= FIELD_HEIGHT || (ny >= 0 && field[ny][nx] == 1)) {
                    can_move = 0;
                }
            }
        }
    }
    if (can_move) {
        falling_block_y++;
    } else {
        fix_falling_block();
        falling_block_active = 0;
        // (필요시 여기서 field에 1로 고정하는 함수 호출 가능)
    }
}

// 좌우 이동
int try_move_block(int dx) {
    int can_move = 1;
    for (int by = 0; by < 3; by++) {
        for (int bx = 0; bx < 3; bx++) {
            if (tetro3x3[falling_block_type][by][bx] == 2) {
                int nx = falling_block_x + bx + dx;
                int ny = falling_block_y + by;
                if (nx < 0 || nx >= FIELD_WIDTH) can_move = 0;
                else if (ny >= 0 && ny < FIELD_HEIGHT && field[ny][nx] == 1) can_move = 0;
            }
        }
    }
    if (can_move) falling_block_x += dx;
    return can_move;
}
void hard_drop(void) {
    while (falling_block_active) {
        move_block_down();
    }
}

// 필드에 고정된 블록을 추가하는 함수
void fix_falling_block(void) {
    // 현재 falling_block_x, falling_block_y, falling_block_type이 사용됨
    for (int by = 0; by < 3; ++by) {
        for (int bx = 0; bx < 3; ++bx) {
            if (tetro3x3[falling_block_type][by][bx] == 2) {
                int fx = falling_block_x + bx;
                int fy = falling_block_y + by;
                // 필드 범위 내에 있을 때만
                if (fx >= 0 && fx < FIELD_WIDTH && fy >= 0 && fy < FIELD_HEIGHT) {
                    field[fy][fx] = 1; // 1 = 고정된 블록
                }
            }
        }
    }
}

// 게임 오버 판정
int is_game_over(void) {
    // 새 블록이 스폰될 좌표 계산
    int offset = get_top_offset(next_block_type); // 또는 falling_block_type
    int spawn_x = 2;
    int spawn_y = -offset;

    for (int by = 0; by < 3; ++by) {
        for (int bx = 0; bx < 3; ++bx) {
            if (tetro3x3[next_block_type][by][bx] == 2) {
                int fx = spawn_x + bx;
                int fy = spawn_y + by;
                // **fy가 0 이상일 때만 검사(음수는 필드 바깥이므로 무시)**
                if (fy >= 0 && fx >= 0 && fx < FIELD_WIDTH && fy < FIELD_HEIGHT) {
                    if (field[fy][fx] == 1)
                        return 1; // 게임오버
                }
            }
        }
    }
    return 0; // 정상
}

// 도트 매트릭스 배열을 채워주는 함수, 실제 fpga에 전달할 배열
void get_display_buffer(int buffer[FIELD_HEIGHT][FIELD_WIDTH]) {
    // 1. 고정 블록 복사
    for (int y = 0; y < FIELD_HEIGHT; y++)
        for (int x = 0; x < FIELD_WIDTH; x++)
            buffer[y][x] = field[y][x];

    // 2. 떨어지는 블록 overlay
    if (falling_block_active) {
        for (int by = 0; by < 3; by++) {
            for (int bx = 0; bx < 3; bx++) {
                if (tetro3x3[falling_block_type][by][bx] == 2) {
                    int fx = falling_block_x + bx;
                    int fy = falling_block_y + by;
                    if (fx >= 0 && fx < FIELD_WIDTH && fy >= 0 && fy < FIELD_HEIGHT)
                        buffer[fy][fx] = 2; // 2는 떨어지는 블록(또는 원하는 숫자)
                }
            }
        }
    }
}
