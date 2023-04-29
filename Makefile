CC = gcc
CFLAGS = -g 

all: main

main: 
	@$(CC) $(CFLAGS) -o main main.c

clean:
	@rm -f *.o main 
    