LDRM_FLAGS = $(shell pkg-config --cflags --libs libdrm)

all:
	gcc -o test test.c -I.. $(LDRM_FLAGS) -lm -g

run: all
	./test
