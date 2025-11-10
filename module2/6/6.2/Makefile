CC = gcc
CFLAGS = -Wall -Werror -Wextra -c
DFLAGS = -shared

all: contact_manager.exe

contact_manager.exe: main.o liblist.dll
	$(CC) main.o -L. -llist -o contact_manager.exe

liblist.dll: list.o
	$(CC) $(DFLAGS) -o liblist.dll list.o -Wl,--out-implib,liblist.a

main.o: main.c list.h
	$(CC) $(CFLAGS) main.c -o main.o

list.o: list.c list.h
	$(CC) $(CFLAGS) list.c -o list.o

clean:
	del /Q *.o *.dll *.exe *.a 2>nul