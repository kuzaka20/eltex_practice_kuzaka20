CC = gcc
CFLAGS = -Wall -Werror -Wextra
TARGET = sniffer

all: $(TARGET)

$(TARGET): sniffer.c
	$(CC) $(CFLAGS) sniffer.c -o $(TARGET)

clean:
	rm -f $(TARGET) packet_dump.bin packet_analysis.txt

run: $(TARGET)
	sudo ./$(TARGET)

.PHONY: all clean run