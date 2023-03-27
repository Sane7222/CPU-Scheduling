scheduler: scheduler.o proc-queues.o
	gcc -o exec scheduler.o proc-queues.o -Wall -Werror

scheduler.o: scheduler.c
	gcc -c scheduler.c

proc-queues.o: proc-queues.c
	gcc -c proc-queues.c

run:
	./exec

clean:
	rm *.o exec