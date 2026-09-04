CC = gcc
CFLAGS = -g -O0 -Wall -Wextra

BIN_DIR = bin
TARGET = $(BIN_DIR)/challenge01
SOURCE = challenge01/challenge01.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

clean:
	rm -rf $(BIN_DIR)

debug: $(TARGET)
	gdb ./$(TARGET)