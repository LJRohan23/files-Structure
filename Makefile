all : project
project : miniproject.o files.o queue.o
	cc miniproject.o files.o queue.o -o project
miniproject.o : miniproject.c files.h queue.h
	cc -c miniproject.c
files.o : files.c files.h queue.h
	cc -c files.c
queue.o : queue.c queue.h
	cc -c queue.c
clean:
	rm *.o
