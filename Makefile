CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
SRC_DIR = src
OBJ_DIR = obj
BUILD_DIR = build

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

TARGET = $(BUILD_DIR)/analyzer

all: $(TARGET)

$(TARGET): $(OBJ_FILES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	@mkdir $(OBJ_DIR) 2>nul || true

$(BUILD_DIR):
	@mkdir $(BUILD_DIR) 2>nul || true

clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR) $(BUILD_DIR) 2>nul || rmdir /s /q $(OBJ_DIR) $(BUILD_DIR) 2>nul
