CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -lm
DLFLAGS = -ldl

all: libmath.so static dynamic

libmath.so: integral.o area.o
	$(CC) -shared -o $@ $^ $(LDFLAGS)

integral.o: integral.c integral.h
	$(CC) $(CFLAGS) -c integral.c

area.o: area.c area.h
	$(CC) $(CFLAGS) -c area.c

static: static.c integral.o area.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

dynamic: dynamic.c
	$(CC) $(CFLAGS) -o $@ dynamic.c $(DLFLAGS)

clean:
	rm -f *.o *.so static dynamic

run_static: static
	./static

run_dynamic: libmath.so dynamic
	./dynamic

.PHONY: all clean run_static run_dynamic