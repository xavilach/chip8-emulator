chip8-emulator: main.o
	gcc -o chip8-emulator main.o

main.o: main.c
	gcc -o main.o -c main.c -W -Wall -ansi -pedantic