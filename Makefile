CC=gcc
CFLAGS=-I./include -Wall -Wextra -g
LDFLAGS=
BINDIR=./bin
SRCDIR=./src
LIBDIR=./lib
OBJDIR=$(LIBDIR)/obj

# Automatically find common source files
COMMON_SOURCES := $(wildcard $(SRCDIR)/common/*/*.c)
COMMON_OBJECTS := $(COMMON_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Automatically find app source files
APP_SOURCES := $(wildcard $(SRCDIR)/apps/*.c)
APP_OBJECTS := $(APP_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
APP_EXECUTABLES := $(patsubst $(SRCDIR)/apps/%.c, $(BINDIR)/%, $(APP_SOURCES))

# All targets
all: $(APP_EXECUTABLES) $(BINDIR)/services

# Compile app
$(BINDIR)/%: $(OBJDIR)/apps/%.o $(COMMON_OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

# Compile app object files
$(OBJDIR)/apps/%.o: $(SRCDIR)/apps/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile service
$(BINDIR)/services: $(OBJDIR)/services/service.o $(COMMON_OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

# Generic rule for compiling source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(BINDIR)/*
	rm -rf $(LIBDIR)

.PHONY: all clean

