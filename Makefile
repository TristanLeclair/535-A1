# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -Iinclude -g

# Directories
SRC_DIR := src
INCLUDE_DIR := include
BIN_DIR := bin
LIB_DIR := lib

APP_SRCS := $(SRC_DIR)/app.c $(SRC_DIR)/multicast.c $(SRC_DIR)/zcs.c
APP_OBJS := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(APP_SRCS))
APP_EXECUTABLE := $(BIN_DIR)/myapp

SERVICE_SRCS := $(SRC_DIR)/service.c  $(SRC_DIR)/multicast.c $(SRC_DIR)/zcs.c
SERVICE_OBJS := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SERVICE_SRCS))
SERVICE_EXECUTABLE := $(BIN_DIR)/myservice

# Targets
all: $(APP_EXECUTABLE) $(SERVICE_EXECUTABLE)

$(APP_EXECUTABLE): $(APP_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(SERVICE_EXECUTABLE): $(SERVICE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR)/*.o $(LIB_DIR)/*.so $(APP_EXECUTABLE) $(SERVICE_EXECUTABLE)

.PHONY: all clean

