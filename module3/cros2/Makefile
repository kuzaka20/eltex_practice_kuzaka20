CC = gcc
CFLAGS = -Wall -O2
TARGET = taxi
SRC = taxi.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
	rm -f /tmp/driver_*.fifo

.PHONY: all clean
