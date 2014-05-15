# Make all! #
all: main prog1 prog2 prog3 prog4 clean

# Main program along with libraries #
main: main.o list.o
	gcc main.o list.o -o main

main.o: main.c list.h
	gcc -Wall -c main.c

list.o: list.c list.h
	gcc -Wall -c list.c

# Testing programs #
prog1: prog1.c
	gcc -Wall prog1.c -o prog1

prog2: prog2.c
	gcc -Wall prog2.c -o prog2

prog3: prog3.c
	gcc -Wall prog3.c -o prog3

prog4: prog4.c
	gcc -Wall prog4.c -o prog4

clean:
	rm -rvf *.o
