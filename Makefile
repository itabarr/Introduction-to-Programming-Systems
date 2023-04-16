CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g # added -Wextra flag for better programming practices
OBJS = main.o process_manager.o

all: hw1shell

hw1shell: $(OBJS)
	@$(CC) $(CFLAGS) -o hw1shell main.o process_manager.o

main.o: main.c process_manager.h
	@$(CC) $(CFLAGS) -c -o main.o main.c

process_manager.o: process_manager.c process_manager.h    
	@$(CC) $(CFLAGS) -c -o process_manager.o process_manager.c 

clean:
	@rm -f *.o hw1shell 
