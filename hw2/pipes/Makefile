#Giorgos Gougoudis 
#Kostas   Kanellis 
#-------------------
#hw2 Makefile
# @ is to mute echoing of commands
#Usage: cd to current dir and execute "make"

#compiler, flags
compiler = @gcc
flags = -Wall

#startup command
all : P1 P2 P3 main clean
	@echo "INF-UTH-2014!!"

#-----------------Main binaries--------------------#
main : libutil.a util.h main.c
	$(compiler) $(flags) -o main main.c -L. -lutil

P3 : libutil.a util.h P1.c
	$(compiler) $((flags) -o P3 P3.c -L. -lutil

P2 : libutil.a util.h P2.c
	$(compiler) $(flags) -o P2 P2.c -L. -lutil

P1 : libutil.a util.h P1.c
	$(compiler) $(flags) -o P1 P1.c -L. -lutil
#--------------------------------------------------#

#make library
util.o : util.c util.h
	$(compiler) $(flags) -c util.c

#insert library into archive
libutil.a : util.o
	@ar cr libutil.a util.o

#cleanup
clean:
	@echo "Building done. Cleaning up!"
	@rm -rf util.o libutil.a

