# Flags
GCC = gcc
# Compilation flags
# -Wall: Enable all warnings
# -Wextra: Enable extra warnings
# -Werror: Treat warnings as errors
# -Wshadow: Warn when a local variable shadows another variable
# -Wstrict-prototypes: Warn if a function is declared or defined without specifying argument types
# -Wconversion: Warn for implicit type conversions
# -Wcast-align: Warn when a pointer cast increases the required alignment
# -Wpointer-arith: Warn about pointer arithmetic on types that are not explicitly defined as pointers
# -Wformat=2: Enable stricter format checks
# -Wundef: Warn about the use of undefined preprocessor macros
# -Wstrict-aliasing=2: Enable strict aliasing warnings
# -Wbad-function-cast: Warn when casting between function pointers and non-function pointers
# -Wnull-dereference: Warn about dereferencing null pointers
# -Walloc-size-larger-than=1000000: Warn when allocating memory larger than 1MB
# -Wmissing-prototypes: Warn if a global function is defined without a prototype
# -fsanitize=address: Enable AddressSanitizer (detects memory errors)
# -fsanitize=undefined: Enable UndefinedBehaviorSanitizer (detects undefined behavior)
# -g: Include debugging information
GCC_FLAGS = -Wall -Wextra -Werror -Wshadow -Wstrict-prototypes -Wcast-align \
           -Wpointer-arith -Wformat=2 -Wundef -Wstrict-aliasing=2 -Wbad-function-cast \
           -Wnull-dereference -Walloc-size-larger-than=1000000 -Wmissing-prototypes \
           -fsanitize=address -fsanitize=undefined -g -DDEBUG 
LD_FLAGS = -fsanitize=address -fsanitize=undefined

# Directories
SRC_DIR = src
OBJ_DIR = obj

# Files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Targets
all: $(OBJ_DIR) $(OBJ_FILES)
	$(GCC) $(LD_FLAGS) $(OBJ_FILES) -o main

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(GCC) $(GCC_FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) main


#List all C files
list:
	@echo $(SRC_FILES)

run:
	./main
.PHONY: all clean
