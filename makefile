# Root Makefile to run Makefiles in scripts/build and scripts/test with access to src

# Define the source directory
SRC_DIR=../../src
OBJ_DIR=../../obj
EXEC_TARGET=../../main
LIB_TARGET=../../cmalloc.so
# Default target
all: build

# Run the build Makefile, passing the SRC_DIR variable
build:
	@$(MAKE) -C scripts/build SRC_DIR=$(SRC_DIR) OBJ_DIR=$(OBJ_DIR) LIB_TARGET=$(LIB_TARGET)

# Run the test Makefile, passing the SRC_DIR variable
test:
	@$(MAKE) -C scripts/test SRC_DIR=$(SRC_DIR) OBJ_DIR=$(OBJ_DIR) EXEC_TARGET=$(EXEC_TARGET)

# Clean both build and test outputs
clean:
	@$(MAKE) -C scripts/build clean SRC_DIR=$(SRC_DIR) OBJ_DIR=$(OBJ_DIR) LIB_TARGET=$(LIB_TARGET)
	@$(MAKE) -C scripts/test clean SRC_DIR=$(SRC_DIR) OBJ_DIR=$(OBJ_DIR) EXEC_TARGET=$(EXEC_TARGET)

.PHONY: all build test clean
