# Flags
GCC = gcc

# Compilation flags
GCC_FLAGS = -Wall -Wextra -Werror -Wshadow -Wstrict-prototypes -Wcast-align \
           -Wpointer-arith -Wformat=2 -Wundef -Wstrict-aliasing=2 -Wbad-function-cast \
           -Wnull-dereference -Walloc-size-larger-than=1000000 -Wmissing-prototypes \
           -fsanitize=address -fsanitize=undefined -g -DDEBUG 

LD_FLAGS = -fsanitize=address -fsanitize=undefined

# Directories
SRC_DIR = src
OBJ_DIR = obj

# Target info
TARGET = main

# Files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Default target
all: $(TARGET)

# Link object files to create the final executable
$(TARGET): $(OBJ_FILES)
	$(GCC) $(OBJ_FILES) $(LD_FLAGS) -o $(TARGET)

# Compile each .c file into a .o file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(GCC) $(GCC_FLAGS) -c $< -o $@

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# List all C files
list:
	@echo $(SRC_FILES)

# Run the executable
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean list run
