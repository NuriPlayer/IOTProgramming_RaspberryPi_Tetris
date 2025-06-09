#include <stdio.h>
#include <windows.h>
#include "tetris_logic.h"

// 입출력용 함수
void clear_screen() {
    system("cls");
}

// buffer 배열을 이용한 출력
void render_field(void) {
    int buffer[FIELD_HEIGHT][FIELD_WIDTH];
    get_display_buffer(buffer); // 로직에서 현재 상태를 받아옴
    
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            if (buffer[y][x] == 1)
                printf("1 "); // 고정 블록
            else if (buffer[y][x] == 2)
                printf("2 "); // 떨어지는 블록
            else
                printf("0 "); // 빈 칸
        }
        printf("\n");
    }
    printf("\n");
}


// 다음 블록 3x3 미리보기
void print_next_block_preview(void) {
    printf("Next Block:\n");
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (tetro3x3[next_block_type][y][x] == 2) printf("2 ");
            else printf("0 ");
        }
        printf("\n");
    }
    printf("\n");
}
