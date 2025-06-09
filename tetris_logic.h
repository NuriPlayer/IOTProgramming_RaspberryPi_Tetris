#ifndef TETRIS_LOGIC_H
#define TETRIS_LOGIC_H

#define FIELD_WIDTH 7     // 도트 매트릭스 열 수
#define FIELD_HEIGHT 10   // 도트 매트릭스 행 수

extern int field[FIELD_HEIGHT][FIELD_WIDTH];  // 게임 필드
extern int next_block_type;
extern int falling_block_x;
extern int falling_block_y;
extern int falling_block_type;
extern int falling_block_active;

extern int tetro3x3[6][3][3];

// 입출력용 함수
void clear_screen(void);
void init_field(void);
void render_field(void);
void generate_next_block(void);
void render_next_block(void);
int is_falling_block_active(void);
int spawn_block(void);
void move_block_down(void);
void print_next_block_preview(void);
int try_move_block(int);
void get_display_buffer(int buffer[FIELD_HEIGHT][FIELD_WIDTH]);
void hard_drop(void);
void fix_falling_block(void);
int is_game_over(void);



#endif