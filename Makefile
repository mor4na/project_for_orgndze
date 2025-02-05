CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = obj

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BUILD_DIR)/analyzer

$(TARGET): $(OBJS)
	@powershell -Command "if (!(Test-Path $(BUILD_DIR))) { New-Item -ItemType Directory -Path $(BUILD_DIR) }"
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@powershell -Command "if (!(Test-Path $(OBJ_DIR))) { New-Item -ItemType Directory -Path $(OBJ_DIR) }"
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@powershell -Command "if (Test-Path $(OBJ_DIR)) { Remove-Item -Recurse -Force $(OBJ_DIR) }"
	@powershell -Command "if (Test-Path $(BUILD_DIR)) { Remove-Item -Recurse -Force $(BUILD_DIR) }"

.PHONY: clean
