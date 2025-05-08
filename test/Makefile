SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LFLAGS  = $(shell sdl2-config --libs)

all:
	gcc -o test test.c -I.. $(SDL2_CFLAGS) $(SDL2_LFLAGS) -lm -g

run: all
	./test