# Makefile for building the shared library

# Flags
GCC = gcc

# Compilation flags
COMMON_FLAGS = -Wall -Wextra -Werror -Wshadow -Wstrict-prototypes -Wcast-align \
               -Wpointer-arith -Wformat=2 -Wundef -Wstrict-aliasing=2 -Wbad-function-cast \
               -Wnull-dereference -Walloc-size-larger-than=1000000 -Wmissing-prototypes
GCC_FLAGS = $(COMMON_FLAGS) -fPIC -DCMALLOC_LIB -DDEBUG
LD_FLAGS = -shared

# Directories
SRC_DIR = src
OBJ_DIR = obj

# Target info
LIB_TARGET = cmalloc.so

# Files
SRC_FILES = $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c))
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Build shared library
all: $(LIB_TARGET)

# Link object files to create the shared library
$(LIB_TARGET): $(OBJ_FILES)
	$(GCC) $(OBJ_FILES) $(LD_FLAGS) -o $(LIB_TARGET)

# Compile each .c file into a .o file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(GCC) $(GCC_FLAGS) -c $< -o $@

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(LIB_TARGET)

.PHONY: all clean
