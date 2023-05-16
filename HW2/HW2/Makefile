all: main

main: 
	@gcc -g -pthread job_queue.c basic_commands.c main.c global_counter_mutex_list.c -o hw2

itamar_test:
	gcc -g -pthread job_queue.c basic_commands.c mock_init.c -o itamar_test

test1:
	gcc -g -pthread job_queue.c basic_commands.c main.c -o hw2

clean:
	@find -type f \( -name 'count*.txt' -o -name 'dispatcher.txt' -o -name 'thread*.txt' -o -name 'stats.txt' \) -delete
	@rm -f *.o main frumkis_test test itamar_test hw2

clean_txt:
	@find -type f \( -name 'count*.txt' -o -name 'dispatcher.txt' -o -name 'thread*.txt' -o -name 'stats.txt' \) -delete
	

