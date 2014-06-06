#ifndef MAIN_H_
#define MAIN_H_

enum {
	DISPLAY_WIDTH = 72,
	DISPLAY_HEIGHT = 64
};

enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_A,
	BUTTON_B,
	BUTTON_START,
	BUTTON_SELECT,
};

void player_gameover(int nr,int lines);
int button_down(unsigned int nr, unsigned int button);
void push_lines(unsigned int nr, unsigned int lines);
int is_occupied(unsigned int nr);
unsigned int rand_int(unsigned int limit);
void pixel(int x, int y, unsigned char color);
void set_frame_buffer(int x, int y, unsigned char color);
void set_frame_buffer_hd(int x, int y, unsigned char color);

#endif
