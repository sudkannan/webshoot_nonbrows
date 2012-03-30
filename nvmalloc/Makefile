CC=gcc
CFLAGS=-I.
DEPS = nv_def.h nv_map.h
OBJ = oswego_malloc.o nv_map.o

all:test
%.o: %.c $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

test: $(OBJ)
	gcc -g test.c -o test $^ $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf test
