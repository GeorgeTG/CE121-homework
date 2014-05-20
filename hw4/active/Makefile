CC = @gcc
FLAGS = -Wall

all: clean
	@echo All done.

receiver: library.h library.o receiver.c
	@echo Building receiver...
	$(CC) $(FLAGS) receiver.c library.o -o receiver

sender: library.h library.o sender.c
	@echo Building sender...
	$(CC) $(FLAGS) sender.c library.o -o sender


library.o: library.h library.c debug.h
	@echo Building library...
	$(CC) $(FLAGS) -c library.c

clean: receiver sender
	@echo Cleaning up....
	@rm -rvf library.o
