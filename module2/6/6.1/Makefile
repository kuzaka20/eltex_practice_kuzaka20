CC = gcc
CFLAGS = -Wall -Werror -Wextra -c

all: contact_manager.exe

contact_manager.exe: main.o liblist.a
	$(CC) main.o -L. -llist -o contact_manager.exe

liblist.a: list.o
	ar rcs liblist.a list.o

main.o: main.c list.h
	$(CC) $(CFLAGS) main.c -o main.o

list.o: list.c list.h
	$(CC) $(CFLAGS) list.c -o list.o

clean:
	del /Q *.o *.a *.exe 2>nul

