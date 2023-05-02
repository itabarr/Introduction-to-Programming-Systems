CC = gcc
CFLAGS = -g 

all: main

main: 
	@$(CC) $(CFLAGS) -o main main.c

itamar_test:
	gcc -g -pthread job_queue.c basic_commands.c mock_init.c -o itamar_test

test1:
	gcc -g -pthread job_queue.c basic_commands.c main.c -o frumkis_test

clean:
	@rm -f *.o main frumkis_test test itamar_test
    