#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#define sdl_support
#ifdef sdl_support
#include <SDL/SDL.h>
#include "sdl_draw/SDL_draw.h"
#endif

//#define serial

#ifdef serial
#include "libftdi1/ftdi.h"
static struct ftdi_context *ftdi;
#endif

#include "main.h"
#include "tetris.h"
#include "grid.h"
#include "tcp.h"
#include "libs/text.h"




enum { ZOOM = 11 };

typedef struct {
	int state;
	int buttons[8];
	char name[30];
} Player;

static Player			players[3];
static unsigned char	display[DISPLAY_HEIGHT][DISPLAY_WIDTH];
static int				rerender = 1;

int button_down(unsigned int nr, unsigned int button) {
	Player* p = &players[nr];

	if(! p->state) {
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

void display_highscore()
{
		
		FILE *hs;

		hs = popen ("cat highscore  | sort -n -r", "r");
		if (!hs)
		{
			fprintf (stderr, "Could not read highscore\n");
		}
		else
		{
			int lines;
			char name[256];
			
			int i = 0;

			print_hd_5x3_at (4,0,"SEKTOR EVOLUTION",15);
			print_hd_5x3_at (0,7,"La Fiesta Grande V",10);
			print_hd_5x3_at (18,14,"HIGHSCORE",15);

			while ((fscanf( hs, "%i : %s\n", &lines, &name[0] ) == 2)&&(i++ < 7)) 
			{	

				print_hd_unsigned_5x3_at (1,17+((i)*6),lines,3, ' ',15);
				print_hd_5x3_at (15,17+( (i)*6), name,10);
			}
		}


		pclose (hs);
}

void player_gameover(int nr,int lines)
{
	if(lines == 0) return;
	
	FILE *fp;
	fp = fopen("highscore", "a");
	if (fp == NULL) {
		printf("error opening highscore file\n");
	}
	else
	{
		printf("gameover %i %i\n",nr,lines);
		fprintf(fp,"%i :  %s\n",lines,players[nr].name);
		fclose(fp);
	}
}



void set_frame_buffer(int x, int y, unsigned char color) {
	assert(x < DISPLAY_WIDTH/2);
	assert(y < DISPLAY_HEIGHT/2);
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

void set_frame_buffer_hd(int x, int y, unsigned char color) {
	assert(x < DISPLAY_WIDTH);
	assert(y < DISPLAY_HEIGHT);
	assert(color < 16);
	if(display[y][x] != color) {
		rerender = 1;
		display[y][x] = color;
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

/*
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
   }*/

#ifdef serial
void write_frame(void)
{
	unsigned char c=0x23;
	int ret = ftdi_write_data(ftdi, &c,1);
	if (ret < 0)
	{
		fprintf(stderr,"write failed , error %d (%s)\n",ret, ftdi_get_error_string(ftdi));
	}

	static unsigned char buf[DISPLAY_HEIGHT*DISPLAY_WIDTH];

	uint32_t pixel=0;

	for(uint8_t y = 0;y<DISPLAY_HEIGHT;y++)
	{
		for(uint8_t x = 0;x<DISPLAY_WIDTH/2;x++)
		{
			uint8_t y2=y;
			if(y2>=32){y2-=32;} else{ y2+=32;};
			buf[pixel] = display[y2][x*2+1]*16+display[y2][x*2];
			pixel++;
		}
	}


		
	ret = ftdi_write_data(ftdi, buf, 2304);
	if (ret < 0)
	{
		fprintf(stderr,"write failed , error %d (%s)\n",ret, ftdi_get_error_string(ftdi));
	}
	usleep(2000);

}
#endif
	
	
void clear_display()
{
	for(uint8_t y = 0;y<DISPLAY_HEIGHT;y++)
	{
		for(uint8_t x = 0;x<DISPLAY_WIDTH;x++)
		{
			display[y][x]=0;
		}
	}
}


int main(int argc, char *argv[]) {
#ifdef sdl_support
	srand(SDL_GetTicks());
#endif
	tetris_load();
	for(int i = 0; i <= 2;i++)
	{
		players[i].state=1;
	}

	//tcpinit();


#ifdef serial

	
	
	
	int ret;
	struct ftdi_version_info version;
	if ((ftdi = ftdi_new()) == 0)
	{
		fprintf(stderr, "ftdi_new failed\n");
		return EXIT_FAILURE;
	}
	version = ftdi_get_library_version();
	printf("Initialized libftdi %s (major: %d, minor: %d, micro: %d, snapshot ver: %s)\n",
			version.version_str, version.major, version.minor, version.micro,
			version.snapshot_str);
	if ((ret = ftdi_usb_open(ftdi, 0x0403, 0x6001)) < 0)
	{
		fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
		ftdi_free(ftdi);
		return EXIT_FAILURE;
	}
	// Read out FTDIChip-ID of R type chips
	if (ftdi->type == TYPE_R)
	{
		unsigned int chipid;
		printf("ftdi_read_chipid: %d\n", ftdi_read_chipid(ftdi, &chipid));
		printf("FTDI chipid: %X\n", chipid);
	}
	ret = ftdi_set_line_property(ftdi, 8, STOP_BIT_1, NONE);
	if (ret < 0)
	{
		fprintf(stderr, "unable to set line parameters: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
		exit(-1);
	}
	ret = ftdi_set_baudrate(ftdi, 500000);
	if (ret < 0)
	{
		fprintf(stderr, "unable to set baudrate: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
		exit(-1);
	}
	
		
	unsigned char c=66;
	ret = ftdi_write_data(ftdi, &c,1);
	c=0;
	ret = ftdi_write_data(ftdi, &c,1);
	ret = ftdi_write_data(ftdi, &c,1);
	ret = ftdi_write_data(ftdi, &c,1);
	if (ret < 0)
	{
		fprintf(stderr,"write failed , error %d (%s)\n",ret, ftdi_get_error_string(ftdi));
	}
	usleep(200);
#endif

#ifdef sdl_support
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
#endif

	int running = 1;
	struct timeval last_player_time,current_time;
		
	gettimeofday(&last_player_time, NULL);
				
	int playercount = 0;

	while(running) {

		char *data;
		//data=tcphandle();

		/*if(data != NULL)
		{
			// handle button codes
			//


			if(data[0]==255)
			{
				unsigned int nr = data[5]-49;

				if(nr < 3)
				{

					for(int i = 0;i<29;i++)
					{
						players[nr].name[i]=(char)data[i+6];
						if(data[i+6] == 64)
						{
							players[nr].name[i]=0;
							i=30;
						}
					}
					//printf("received name %i %s\n",nr,players[nr].name);
				}
			}
			if(data[0]==0)
			{
				//printf("received button data %i %i %i\n",data[1],data[2],data[3]);

				playercount=0;

				for(int i = 0; i <= 2; i++)
				{


					if((data[i+1]&16)==16)
					{	
						playercount++;
						if(players[i].state)
						{
							reset_player(i);
							players[i].state = 0;
						}

						if((data[i+1]&2)==2)
						{
							players[i].buttons[2]=1;
						}
						else
						{
							players[i].buttons[2]=0;
						}

						if((data[i+1]&4)==4)
						{
							players[i].buttons[3]=1;
						}
						else
						{
							players[i].buttons[3]=0;
						}

						if((data[i+1]&8)==8)
						{
							players[i].buttons[1]=1;
						}
						else
						{
							players[i].buttons[1]=0;
						}

						if((data[i+1]&1)==1)
						{
							players[i].buttons[4]=1;
						}
						else
						{
							players[i].buttons[4]=0;
						}
					}
					else
					{
						if(! players[i].state)
						{
							//printf("player off\n");
							reset_player(i);
							players[i].state = 1;
						}
					}
				}

			}

		}
		*/


#ifdef sdl_support
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
							display_highscore();
							break;
						case SDLK_2:
							display_highscore();
							//tetris_suspend();
							break;




						default: break;
					}
				default: break;
			}
		}
#endif

		if(playercount != 0)
		{
			gettimeofday(&last_player_time, NULL);
		}
		else
		{
			gettimeofday(&current_time, NULL);
		}
	
		if((current_time.tv_sec-last_player_time.tv_sec) > 70)
		{
			clear_display();
			gettimeofday(&last_player_time, NULL);
		}
		else if((current_time.tv_sec-last_player_time.tv_sec) > 6)
		{
			//clear_display();
			//display_highscore();
		}
		else
		{
			tetris_update();
		}

		if(rerender) {
			rerender = 0;
#ifdef sdl_support
			for(int x = 0; x < DISPLAY_WIDTH; x++)
				for(int y = 0; y < DISPLAY_HEIGHT; y++)
					Draw_FillCircle(screen, ZOOM * x + ZOOM / 2,
							ZOOM * y + ZOOM / 2, ZOOM * 0.45, COLORS[display[y][x]]);
			SDL_Flip(screen);
#endif
#ifdef serial
			write_frame();
#endif
		}
#ifdef sdl_support
		SDL_Delay(10);
#else
		usleep(20000);
#endif
	}

#ifdef sdl_support
	SDL_Quit();
#endif
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

