CC = gcc
CFLAGS = -g -O0 -Wall -Wextra

BIN_DIR = bin

TARGET01 = $(BIN_DIR)/challenge01
SOURCE01 = challenge01/challenge01.c

TARGET02 = $(BIN_DIR)/challenge02
SOURCE02 = challenge02/challenge02.c

TARGET03 = $(BIN_DIR)/challenge03
SOURCE03 = challenge03/challenge03.c

all: $(TARGET01) $(TARGET02) $(TARGET03)

$(TARGET01): $(SOURCE01)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SOURCE01) -o $(TARGET01)

$(TARGET02): $(SOURCE02)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SOURCE02) -o $(TARGET02)

$(TARGET03): $(SOURCE03)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SOURCE03) -o $(TARGET03)

clean:
	rm -rf $(BIN_DIR)

debug01: $(TARGET01)
	gdb ./$(TARGET01)

debug02: $(TARGET02)
	gdb ./$(TARGET02)

debug03: $(TARGET03)
	gdb ./$(TARGET03)