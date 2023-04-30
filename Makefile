CC = gcc
CFLAGS = -g 

all: main

main: 
	@$(CC) $(CFLAGS) -o main main.c

test:
	gcc -g job_queue.c basic_commands.c -o test

clean:
	@rm -f *.o main 
    