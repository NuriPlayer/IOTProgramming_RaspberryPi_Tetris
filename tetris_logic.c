#include "tetris_logic.h"
#include <stdio.h>
#include <stdlib.h>

// 게임판
int field[FIELD_HEIGHT][FIELD_WIDTH];
int next_block_type;

// 떨어지는 블록 정보
int falling_block_x;
int falling_block_y;
int falling_block_type;
int falling_block_active;

// 현재 떨어지는 블록의 크기 (3x3 또는 4x4)
int falling_block_size;

// 3블록 테트로미노 (회전 포함, 3x3)
int tetro3x3[6][3][3] = {
    {
        {0,0,0},
        {2,2,2},
        {0,0,0}
    }, // 가로 일자
    {
        {0,2,0},
        {0,2,0},
        {0,2,0}
    }, // 세로 일자
    {
        {0,2,0},
        {0,2,2},
        {0,0,0}
    }, // ㄴ (왼쪽 아래)
    {
        {0,2,0},
        {2,2,0},
        {0,0,0}
    }, // ㄴ (오른쪽 아래)
    {
        {0,0,0},
        {0,2,2},
        {0,2,0}
    }, // ㄴ (왼쪽 위)
    {
        {0,0,0},
        {2,2,0},
        {0,2,0}
    }, // ㄴ (오른쪽 위)
};
// 4블록 테트로미노 (4x4)
int tetro4x4[6][4][4] = {
    // ㅡ 블록 (일자형)
    {
        {0, 0, 0, 0},
        {2, 2, 2, 2},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // ㅁ 블록 (정사각형)
    {
        {0, 0, 0, 0},
        {0, 2, 2, 0},
        {0, 2, 2, 0},
        {0, 0, 0, 0}
    },
    // ㄴ 블록
    {
        {0, 0, 0, 0},
        {0, 0, 2, 0},
        {2, 2, 2, 0},
        {0, 0, 0, 0}
    },
    // ㅗ 블록
    {
        {0, 0, 0, 0},
        {0, 2, 0, 0},
        {2, 2, 2, 0},
        {0, 0, 0, 0}
    },
    // ㄹ 블록
    {
        {0, 0, 0, 0},
        {2, 2, 0, 0},
        {0, 2, 2, 0},
        {0, 0, 0, 0}
    },
    // ㄹ 블록 (반대)
    {
        {0, 0, 0, 0},
        {0, 0, 2, 2},
        {0, 2, 2, 0},
        {0, 0, 0, 0}
    }
};


// 난이도(0 ~ 4)
int difficulty = 0;
// 점수(FND 표기용)
int score = 0;
// 블록 고정 이벤트
int block_fixed_flag = 0;
void add_score(int lines_cleared) {
    int add = 0;
    switch (lines_cleared) {
        case 1: add = 10 + difficulty * 1; break;
        case 2: add = 20 + difficulty * 2; break;
        case 3: add = 30 + difficulty * 3; break;
        case 4: add = 40 + difficulty * 4; break;
    }
    score += add;
    if (score > 9999) score = 9999;
}

int get_top_offset(int type) {
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

int get_top_offset_4x4(int type) {
    int offset = 0;
    for (int y = 0; y < 4; y++) {
        int empty = 1;
        for (int x = 0; x < 4; x++) {
            if (tetro4x4[type][y][x] == 2) {
                empty = 0;
                break;
            }
        }
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
    next_block_type = rand() % 12;
}

int spawn_block(void) {
    int is_4x4 = (next_block_type >= 6);
    falling_block_type = is_4x4 ? next_block_type - 6 : next_block_type;
    falling_block_size = is_4x4 ? 4 : 3;

    int offset = is_4x4 ? get_top_offset_4x4(falling_block_type)
                        : get_top_offset(falling_block_type);

    falling_block_x = (FIELD_WIDTH - falling_block_size) / 2;
    falling_block_y = -offset;
    falling_block_active = 1;

    generate_next_block(); // 다음 블록 갱신

    if (is_game_over()) {
        falling_block_active = 0;
        return 0;
    }
    return 1;
}



// 충돌판정/자동 고정
void move_block_down(void) {
    int can_move = 1;
    int size = falling_block_size;
    for (int by = 0; by < size; by++) {
        for (int bx = 0; bx < size; bx++) {
            int block_val = (size == 3) ? tetro3x3[falling_block_type][by][bx]
                                        : tetro4x4[falling_block_type][by][bx];
            if (block_val == 2) {
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
    }
}

// 좌우 이동
int try_move_block(int dx) {
    int can_move = 1;
    int size = falling_block_size;

    for (int by = 0; by < size; ++by) {
        for (int bx = 0; bx < size; ++bx) {
            int val = (size == 3) ? tetro3x3[falling_block_type][by][bx]
                                  : tetro4x4[falling_block_type][by][bx];
            if (val == 2) {
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
// 하드 드롭 함수
void hard_drop(void) {
    while (falling_block_active) {
        move_block_down();
    }
}
// N×N 매트릭스 시계방향 90도 회전 (matrix 본체를 직접 변경)
static void rotate_matrix(int matrix[4][4], int size) {
    int tmp[4][4] = {0};
    // tmp[x][size-1-y] = matrix[y][x]
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            tmp[x][size - 1 - y] = matrix[y][x];
        }
    }
    // 다시 원본으로 복사
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            matrix[y][x] = tmp[y][x];
        }
    }
}

// falling_block_size, falling_block_type을 보고 현재 블록 회전
void rotate_falling_block(void) {
    if (!falling_block_active) return;

    int size = falling_block_size;
    // 임시 배열에 블록 상태 복사
    int buf[4][4] = {0};
    if (size == 3) {
        for (int y = 0; y < 3; y++)
            for (int x = 0; x < 3; x++)
                buf[y][x] = tetro3x3[falling_block_type][y][x];
    } else {
        for (int y = 0; y < 4; y++)
            for (int x = 0; x < 4; x++)
                buf[y][x] = tetro4x4[falling_block_type][y][x];
    }

    // 회전
    rotate_matrix(buf, size);

    // 회전 후 충돌 검사: 회전된 buf를 falling_block 위치에 놓아도 문제가 없을 때만 적용
    int can_rotate = 1;
    for (int y = 0; y < size && can_rotate; y++) {
        for (int x = 0; x < size; x++) {
            if (buf[y][x] == 2) {
                int fx = falling_block_x + x;
                int fy = falling_block_y + y;
                // 벽 또는 이미 고정된 블록과 충돌하면 회전 취소
                if (fx < 0 || fx >= FIELD_WIDTH ||
                    fy >= FIELD_HEIGHT ||
                    (fy >= 0 && field[fy][fx] == 1)) {
                    can_rotate = 0;
                    break;
                }
            }
        }
    }

    // 충돌 없으면 실제 배열에 덮어쓰기
    if (can_rotate) {
        if (size == 3) {
            for (int y = 0; y < 3; y++)
                for (int x = 0; x < 3; x++)
                    tetro3x3[falling_block_type][y][x] = buf[y][x];
        } else {
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                    tetro4x4[falling_block_type][y][x] = buf[y][x];
        }
    }
}

// 필드에 고정된 블록을 추가하는 함수
void fix_falling_block(void) {
    int size = falling_block_size;
    for (int by = 0; by < size; ++by) {
        for (int bx = 0; bx < size; ++bx) {
            int val = (size == 3) ? tetro3x3[falling_block_type][by][bx]
                                  : tetro4x4[falling_block_type][by][bx];
            if (val == 2) {
                int fx = falling_block_x + bx;
                int fy = falling_block_y + by;
                if (fx >= 0 && fx < FIELD_WIDTH && fy >= 0 && fy < FIELD_HEIGHT) {
                    field[fy][fx] = 1;
                }
            }
        }
    }

    int lines = clear_full_lines();
    if (lines > 0) {
        add_score(lines);
    }
    block_fixed_flag = 1;
}

// 한 행에 1(고정블록)이 가득 차면 제거하는 함수
int clear_full_lines(void) {
    int lines_cleared = 0;

    for (int y = FIELD_HEIGHT - 1; y >= 0; --y) {
        int full = 1;
        for (int x = 0; x < FIELD_WIDTH; ++x) {
            if (field[y][x] == 0) {
                full = 0;
                break;
            }
        }
        if (full) {
            // 아래로 한 칸씩 내리기
            for (int k = y; k > 0; --k) {
                for (int x = 0; x < FIELD_WIDTH; ++x)
                    field[k][x] = field[k-1][x];
            }
            // 맨 위 행을 비움
            for (int x = 0; x < FIELD_WIDTH; ++x)
                field[0][x] = 0;
            ++lines_cleared;
            ++y; // 다시 같은 y(내려온 행)를 검사(여러 줄 연속 삭제 가능)
        }
    }
    return lines_cleared;
}

// 게임 오버 판정
int is_game_over(void) {
    int size = falling_block_size;
    int spawn_x = falling_block_x;
    int spawn_y = falling_block_y;

    for (int by = 0; by < size; ++by) {
        for (int bx = 0; bx < size; ++bx) {
            int val = (size == 3) ? tetro3x3[falling_block_type][by][bx]
                                  : tetro4x4[falling_block_type][by][bx];
            if (val == 2) {
                int fx = spawn_x + bx;
                int fy = spawn_y + by;
                if (fy >= 0 && fy < FIELD_HEIGHT && fx >= 0 && fx < FIELD_WIDTH) {
                    if (field[fy][fx] == 1) return 1;
                }
            }
        }
    }
    return 0;
}

// 도트 매트릭스 배열을 채워주는 함수, 실제 fpga에 전달할 배열
void get_display_buffer(int buffer[FIELD_HEIGHT][FIELD_WIDTH]) {
    for (int y = 0; y < FIELD_HEIGHT; y++)
        for (int x = 0; x < FIELD_WIDTH; x++)
            buffer[y][x] = field[y][x];

    if (falling_block_active) {
        int size = falling_block_size;
        for (int by = 0; by < size; ++by) {
            for (int bx = 0; bx < size; ++bx) {
                int val = (size == 3) ? tetro3x3[falling_block_type][by][bx]
                                      : tetro4x4[falling_block_type][by][bx];
                if (val == 2) {
                    int fx = falling_block_x + bx;
                    int fy = falling_block_y + by;
                    if (fx >= 0 && fx < FIELD_WIDTH && fy >= 0 && fy < FIELD_HEIGHT)
                        buffer[fy][fx] = 2;
                }
            }
        }
    }
}
