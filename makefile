# Root Makefile to run Makefiles in scripts/build and scripts/test with access to src

# Define the source directory
SRC_DIR=../../src
OBJ_DIR_TESTS=../../obj_tests
OBJ_DIR_LIB=../../obj_build
EXEC_TARGET=../../main
LIB=cmalloc.so
LIB_TARGET=../../$(LIB)

# Default target
all: build-test

# Run the build Makefile, passing the SRC_DIR variable
build-lib:
	@$(MAKE) -C scripts/build SRC_DIR=$(SRC_DIR) OBJ_DIR=$(OBJ_DIR_LIB) LIB_TARGET=$(LIB_TARGET)

# Run the test Makefile, passing the SRC_DIR variable
build-test:
	@$(MAKE) -C scripts/test SRC_DIR=$(SRC_DIR) OBJ_DIR=$(OBJ_DIR_TESTS) EXEC_TARGET=$(EXEC_TARGET)

test:
	@$(MAKE) -C tests 
	@$(MAKE) -C tests run
	# @$(MAKE) -C tests run-ls
nano:$(LIB)
	LD_DEBUG=all LD_PRELOAD=./cmalloc.so nano > debug.txt 2>&1
# Clean both build and test outputs
clean:
	@$(MAKE) -C scripts/build clean SRC_DIR=$(SRC_DIR) OBJ_DIR=$(OBJ_DIR_LIB) LIB_TARGET=$(LIB_TARGET)
	@$(MAKE) -C scripts/test clean SRC_DIR=$(SRC_DIR) OBJ_DIR=$(OBJ_DIR_TESTS) EXEC_TARGET=$(EXEC_TARGET)

.PHONY: all build-lib build-test clean nano
