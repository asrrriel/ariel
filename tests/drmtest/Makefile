LDRM_FLAGS = $(shell pkg-config --cflags --libs libdrm)

all:
	gcc -o test test.c -I../../libbetterm -I../../libdazzle/include -I../../libdazzletype $(LDRM_FLAGS) -lm -g

run: all
	./test
