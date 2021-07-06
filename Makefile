CC = gcc
CFLAGS = -g -Wall
target = main

all: main

main: main.c
	gcc -g -Wall -o main.exe main.c

clean: 
	del "rotated*"