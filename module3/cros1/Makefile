CC      = gcc
CFLAGS  = -Wall -Wextra -O2
LDFLAGS = -pthread

TARGETS = server client

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) server.c -o server $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) client.c -o client $(LDFLAGS)

clean:
	rm -f $(TARGETS)

rebuild: clean all