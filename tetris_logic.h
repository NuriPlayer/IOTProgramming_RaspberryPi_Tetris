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
extern int tetro4x4[6][4][4];

extern int score;  // 현재 점수
extern int difficulty;
extern int falling_block_size; // 현재 떨어지는 블록의 크기 (3x3 또는 4x4)
extern int block_fixed_flag;

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
int clear_full_lines(void);
void add_score(int lines_cleared);
int get_top_offset(int type);
int get_top_offset_4x4(int type);
void rotate_falling_block(void);

#endif