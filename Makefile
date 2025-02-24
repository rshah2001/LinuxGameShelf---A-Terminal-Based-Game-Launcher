CC = gcc
CFLAGS = -Wall -Werror -std=c99
TARGET = shelf-steam

all: $(TARGET)

$(TARGET): shelf-steam.c
	$(CC) $(CFLAGS) -o $(TARGET) shelf-steam.c

clean:
	rm -f $(TARGET)
