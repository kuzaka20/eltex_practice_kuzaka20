CC = gcc
CFLAGS = -Wall -Werror -Wextra -pthread
MATH_LIB = -lm

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o server $(MATH_LIB)

client: client.c
	$(CC) $(CFLAGS) client.c -o client $(MATH_LIB)

clean:
	rm -f server client

run_server: server
	./server

run_client: client
	./client