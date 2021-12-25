CC = g++
CFLAGS = -g -lm -Wall -Wextra

.PHONY: all

all: clean main

main: main.cpp mem.cpp cpu.cpp

clean:
	rm -f main