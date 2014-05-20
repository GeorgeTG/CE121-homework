all: clean
	@echo All done.

receiver: library.h library.o receiver.c
	gcc -Wall receiver.c library.o -o receiver

sender:  library.h library.o sender.c
	gcc -Wall sender.c library.o -o sender


library.o: library.h library.c debug.h
	gcc -Wall -c library.c

clean : receiver sender
	@echo Cleaning up....
	rm -rvf library.o
