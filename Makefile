# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -Iinclude -g
LDFLAGS := -Llib -lmulticast -lzcs

SRC_DIRS = src
BIN_DIR = bin
LIB_DIR = lib

APPS = main app service

# Find all source files
SRC_FILES := $(shell find $(SRC_DIRS) -name '*.c')

# Derive object file names from source files
OBJ_FILES := $(SRC_FILES:%.c=%.o)

# Derive executable names from app names
EXE_FILES := $(APPS:%=$(BIN_DIR)/%)

# Derive library names from lib names
LIB_FILES := $(APPS:%=$(LIB_DIR)/lib%.a)

all: $(EXE_FILES)

# Compile each source file into an object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create static libraries
$(LIB_DIR)/lib%.a: $(SRC_DIRS)/%/common/%/*.c
	@mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) -c $^
	ar rcs $@ $(notdir $(patsubst %.c,%.o,$^))
	rm -f $(notdir $(patsubst %.c,%.o,$^))

# Build each executable
$(BIN_DIR)/%: $(SRC_DIRS)/%/src/*.c $(LIB_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR) $(LIB_DIR) $(OBJ_FILES)
