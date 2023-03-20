scheduler: scheduler.o
	gcc -o exec scheduler.o -Wall -Werror

scheduler.o: scheduler.c
	gcc -c scheduler.c

run:
	./exec

clean:
	rm *.o exec