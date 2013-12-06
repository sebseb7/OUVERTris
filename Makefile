UNAME := $(shell uname)


SRC = $(wildcard *.c)
SRC+= sdl_draw/SDL_draw.c
SRC+= pixelfonts/5x3/font.c
SRC+= $(wildcard libs/*.c)

FLAGS= --std=gnu99 -Wall -funsigned-char -Wundef -Wsign-compare -pedantic  -Wstrict-prototypes


ifeq ($(UNAME), Darwin)
	COMPILER = clang
	FLAGS   += -Qunused-arguments
	
	FLAGS   += `sdl-config --cflags` 
	LDFLAGS += `sdl-config --libs`  
endif

ifeq ($(UNAME), Linux)
    FLAGS +=  -lSDL
endif


all:
	clang $(FLAGS) $(SRC) -o tet -I. $(LDFLAGS)

clean:
	rm -f tet

