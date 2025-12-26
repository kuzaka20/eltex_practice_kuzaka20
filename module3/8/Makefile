CC = gcc
CFLAGS = -Wall -Werror -Wextra

all: main

main:
	$(CC) $(CFLAGS) consumer.c -o consumer
	$(CC) $(CFLAGS) producer.c -o producer
	
clean:
	rm -f consumer producer
