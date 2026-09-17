CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -pedantic -std=c11 -O2 -Iinclude
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET := $(BIN_DIR)/posix-shell
SYMLINK := posix-shell

.PHONY: all clean test

all: $(TARGET) $(SYMLINK)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(SYMLINK): $(TARGET)
	ln -sf $(TARGET) $(SYMLINK)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(SYMLINK) test_out*.txt test_err*.txt

test: all
	@chmod +x tests/test_shell.sh
	@./tests/test_shell.sh
