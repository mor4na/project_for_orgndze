CC = gcc 
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
SRC = src/main.c src/module.c
OBJ = $(SRC:.c=.o)
TARGET = build/analyzer

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(OBJ) $(TARGET)

.PHONY: all clean
