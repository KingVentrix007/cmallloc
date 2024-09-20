# Makefile for building the executable

# Flags
GCC = gcc

# Compilation flags
COMMON_FLAGS = -Wall -Wextra -Werror -Wshadow -Wstrict-prototypes -Wcast-align \
               -Wpointer-arith -Wformat=2 -Wundef -Wstrict-aliasing=2 -Wbad-function-cast \
               -Wnull-dereference -Walloc-size-larger-than=1000000 -Wmissing-prototypes
GCC_FLAGS = $(COMMON_FLAGS) -fsanitize=address -fsanitize=undefined -g -DDEBUG
LD_FLAGS = -fsanitize=address -fsanitize=undefined

# Directories
SRC_DIR = src
OBJ_DIR = obj

# Target info
EXEC_TARGET = ../../main

# Files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Default target: Build and run the executable
all: $(EXEC_TARGET)

# Link object files to create the executable
$(EXEC_TARGET): $(OBJ_FILES)
	$(GCC) $(OBJ_FILES) $(LD_FLAGS) -o $(EXEC_TARGET)

# Compile each .c file into a .o file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(GCC) $(GCC_FLAGS) -c $< -o $@

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(EXEC_TARGET)

.PHONY: all clean
