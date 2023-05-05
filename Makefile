all: main

main: 
	@gcc -g -pthread job_queue.c basic_commands.c main.c -o hw2

itamar_test:
	gcc -g -pthread job_queue.c basic_commands.c mock_init.c -o itamar_test

test1:
	gcc -g -pthread job_queue.c basic_commands.c main.c -o hw2

clean:
	@find . -type f -name "*.txt" ! -name "cmdfile.txt" -delete
	@rm -f *.o main frumkis_test test itamar_test hw2

clean_txt:
	@find . -type f -name "*.txt" ! -name "cmdfile.txt" -delete

