# Makefile for EXT2 Parser - Lab 13
# Author: Fatima

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
TARGET = myfs
SOURCE = ext2_parser.cpp
IMAGE = my_partition.img

# Default target
all: $(TARGET)

# Build the parser
$(TARGET): $(SOURCE)
	@echo "Compiling EXT2 Parser..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)
	@echo "Build successful! Executable: ./$(TARGET)"

# Optimized build
release: CXXFLAGS += -O2
release: clean $(TARGET)
	@echo "Optimized build complete!"

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "Debug build complete!"

# Setup disk image
setup: $(TARGET)
	@echo "Setting up EXT2 disk image..."
	@chmod +x setup_disk.sh
	@./setup_disk.sh

# Test ls command
test-ls: $(TARGET)
	@echo "Testing ls command..."
	@./$(TARGET) $(IMAGE) ls

# Test cp command
test-cp: $(TARGET)
	@echo "Testing cp command..."
	@./$(TARGET) $(IMAGE) cp sample.txt
	@echo "Verifying copied file:"
	@cat sample.txt

# Test info command
test-info: $(TARGET)
	@echo "Testing info command..."
	@./$(TARGET) $(IMAGE) info

# Run all tests
test: test-ls test-cp test-info
	@echo "All tests completed!"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(TARGET)
	@rm -f sample.txt hello.txt data.txt hello.c
	@echo "Clean complete!"

# Clean everything including disk image
clean-all: clean
	@echo "Removing disk image..."
	@rm -f $(IMAGE)
	@echo "All clean!"

# Show help
help:
	@echo "EXT2 Parser - Lab 13 Makefile"
	@echo "============================="
	@echo "Available targets:"
	@echo "  make           - Build the parser"
	@echo "  make setup     - Create and setup disk image"
	@echo "  make test-ls   - Test ls command"
	@echo "  make test-cp   - Test cp command"
	@echo "  make test-info - Test info command"
	@echo "  make test      - Run all tests"
	@echo "  make clean     - Remove build artifacts"
	@echo "  make clean-all - Remove everything including disk"
	@echo "  make help      - Show this help"
	@echo ""
	@echo "Quick start:"
	@echo "  1. make setup    # Creates disk image"
	@echo "  2. make test     # Runs all tests"

.PHONY: all release debug setup test-ls test-cp test-info test clean clean-all help
