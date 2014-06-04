#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
//#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
//#endif

#include "main.h"
#include "tetris.h"
#include "grid.h"
#include "sdl_draw/SDL_draw.h"
#include "tcp.h"

//#define serial


#ifdef serial
static int serial_g3d2;
#endif

enum { ZOOM = 11 };

typedef struct {
	int state;
	int buttons[8];
	char name[32];
} Player;

static Player			players[3];
static int				input_map[3];
static unsigned char	display[DISPLAY_HEIGHT][DISPLAY_WIDTH];
static int				rerender = 1;


/*static void set_button(int input_nr, int button, int state) {

	Player* p = &players[input_nr];
	p->buttons[button] = state;
}
*/
int button_down(unsigned int nr, unsigned int button) {
	Player* p = &players[input_map[nr]];
	if(p->state) {
		return p->buttons[button];
	}
	return 0;
}
int is_occupied(unsigned int nr) {



	if(players[nr].state)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
void push_lines(unsigned int nr, unsigned int lines) {
}


/*static int map_key(unsigned int sdl_key) {
	switch(sdl_key) {
	case SDLK_RIGHT:	return BUTTON_RIGHT;
	case SDLK_LEFT:		return BUTTON_LEFT;
	case SDLK_UP:		return BUTTON_UP;
	case SDLK_DOWN:		return BUTTON_DOWN;
	case SDLK_x:		return BUTTON_A;
	case SDLK_c:		return BUTTON_B;
	case SDLK_RETURN:	return BUTTON_START;
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:	return BUTTON_SELECT;
	default:			return -1;
	}
}*/



void set_frame_buffer(int x, int y, unsigned char color) {
	assert(x < DISPLAY_WIDTH);
	assert(y < DISPLAY_HEIGHT);
	assert(color < 16);
	if(display[y*2][x*2] != color) {
		rerender = 1;
		display[y*2][x*2] = color;
	}
	if(display[y*2][x*2+1] != color) {
		rerender = 1;
		display[y*2][x*2+1] = color;
	}
	if(display[y*2+1][x*2+1] != color) {
		rerender = 1;
		display[y*2+1][x*2+1] = color;
	}
	if(display[y*2+1][x*2] != color) {
		rerender = 1;
		display[y*2+1][x*2] = color;
	}
}
void pixel(int x, int y, unsigned char color) {
	assert(x < DISPLAY_WIDTH);
	assert(y < DISPLAY_HEIGHT);
	assert(color < 16);
	if(display[y][x] != color) {
		rerender = 1;
		display[y][x] = color;
	}
}

uint8_t esc(uint8_t color)
{
   if(color == 0x64)
   	return 3;
   if(color == 0x23)
   	return 1;
   if(color == 0x42)
   	return 2;
   if(color == 0x66)
   	return 4;
   
   return color;
}

#ifdef serial
void write_frame(void)
{
			unsigned char c=0x23;
			write(serial_g3d2,&c,1);


	static unsigned char buf[DISPLAY_HEIGHT*DISPLAY_WIDTH];

	uint32_t pixel=0;

	for(uint8_t y = 0;y<DISPLAY_HEIGHT;y++)
	{
		for(uint8_t x = 0;x<DISPLAY_WIDTH/2;x++)
		{
			uint8_t y2=y;
			if(y2>=32){y2-=32;} else{ y2+=32;};
			buf[pixel] = display[y2][x*2]*16+display[y2][x*2+1];
			pixel++;
		}
	}
		

            write(serial_g3d2,&buf,2304);
            usleep(2000);

}
#endif


int main(int argc, char *argv[]) {
	srand(SDL_GetTicks());
	tetris_load();
	for(int i = 0; i <= 2;i++)
	{
		players[i].state=1;
	}

	tcpinit();
	
	
#ifdef serial
	struct termios config2;
	memset(&config2, 0, sizeof(config2));

	if ( (serial_g3d2=open("/dev/cu.usbserial-A100DEF4", O_RDWR)) == -1)
	{
		printf( "Error %d opening device)\n", errno );
	}
	tcgetattr(serial_g3d2, &config2);
    speed_t speed = 500000;
    if ( ioctl( serial_g3d2,  IOSSIOSPEED, &speed ) == -1 )
    {
        printf( "Error %d calling ioctl( ..., IOSSIOSPEED, ... )\n", errno );
    }
			
			unsigned char c=66;
			write(serial_g3d2,&c,1);
            c=0;
            write(serial_g3d2,&c,1);
            c=0;
            write(serial_g3d2,&c,1);
            c=0;
            write(serial_g3d2,&c,1);
            usleep(200);
#endif

	SDL_Surface* screen = SDL_SetVideoMode(
		DISPLAY_WIDTH * ZOOM,
		DISPLAY_HEIGHT * ZOOM,
		32, SDL_SWSURFACE | SDL_DOUBLEBUF);

	const unsigned int COLORS[] = {
		SDL_MapRGB(screen->format, 0x00,0x10,0x00),
		SDL_MapRGB(screen->format, 0x00,0x20,0x00),
		SDL_MapRGB(screen->format, 0x00,0x30,0x00),
		SDL_MapRGB(screen->format, 0x00,0x40,0x00),
		SDL_MapRGB(screen->format, 0x00,0x50,0x00),
		SDL_MapRGB(screen->format, 0x00,0x60,0x00),
		SDL_MapRGB(screen->format, 0x00,0x70,0x00),
		SDL_MapRGB(screen->format, 0x00,0x80,0x00),
		SDL_MapRGB(screen->format, 0x00,0x90,0x00),
		SDL_MapRGB(screen->format, 0x00,0xa0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xb0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xc0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xd0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xe0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xf0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xff,0x00)
	};

	int running = 1;

	while(running) {

		int *data;
		data=tcphandle();

		if(data != NULL)
		{
			// handle button codes
			//
			printf("received data %i %i %i\n",data[0],data[1],data[2]);
		}


		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {

			switch(ev.type) {
			case SDL_QUIT:
				running = 0;
				break;
			case SDL_KEYUP:
			case SDL_KEYDOWN:

		/*		key = map_key(ev.key.keysym.sym);
				if(key > 0) {
					set_button(input_nr, key, ev.type == SDL_KEYDOWN);
					break;
				}
				if(ev.key.keysym.sym == SDLK_SPACE) {
					fast ^= ev.type == SDL_KEYDOWN;
					break;
				}
		*/
				if(ev.type == SDL_KEYUP) break;

				switch(ev.key.keysym.sym) {
				case SDLK_ESCAPE:
					running = 0;
					break;

				case SDLK_1:
					reset_player(0);
					if(players[0].state)
					{
						players[0].state = 0;
					}
					else
					{
						players[0].state = 1;
					}
					break;
				case SDLK_2:
					tetris_suspend();
					break;




				default: break;
				}
			default: break;
			}
		}

		tetris_update();

		if(rerender) {
			rerender = 0;
			for(int x = 0; x < DISPLAY_WIDTH; x++)
				for(int y = 0; y < DISPLAY_HEIGHT; y++)
					Draw_FillCircle(screen, ZOOM * x + ZOOM / 2,
						ZOOM * y + ZOOM / 2, ZOOM * 0.45, COLORS[display[y][x]]);
			SDL_Flip(screen);
#ifdef serial
			write_frame();
#endif
		}
		SDL_Delay(20);
	}
	
	SDL_Quit();
	return 0;
}

// found it in the internet...
static unsigned int my_rand(void) {
	static unsigned int z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
	unsigned int b;
	b  = ((z1 << 6) ^ z1) >> 13;
	z1 = ((z1 & 4294967294U) << 18) ^ b;
	b  = ((z2 << 2) ^ z2) >> 27; 
	z2 = ((z2 & 4294967288U) << 2) ^ b;
	b  = ((z3 << 13) ^ z3) >> 21;
	z3 = ((z3 & 4294967280U) << 7) ^ b;
	b  = ((z4 << 3) ^ z4) >> 12;
	z4 = ((z4 & 4294967168U) << 13) ^ b;
	unsigned int ret = z1 ^ z2 ^ z3 ^ z4;
	return ret;
}

unsigned int rand_int(unsigned int limit) {
	unsigned int ret = my_rand() % limit;
	return ret;
}

