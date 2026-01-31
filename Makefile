# Makefile for BME688 Linux Library
# Supports building the library and example applications

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I.
LDFLAGS = 

# Directories
BME68X_DIR = bme68x
SRC_DIR = src
EXAMPLES_DIR = examples
BUILD_DIR = build
LIB_DIR = lib

# Source files
BME68X_SRC = $(BME68X_DIR)/bme68x.c
LINUX_SRC = $(SRC_DIR)/bme68x_linux.c
EXAMPLE_SRC = $(EXAMPLES_DIR)/basic_read.c

# Object files
BME68X_OBJ = $(BUILD_DIR)/bme68x.o
LINUX_OBJ = $(BUILD_DIR)/bme68x_linux.o
EXAMPLE_OBJ = $(BUILD_DIR)/basic_read.o

# Library
STATIC_LIB = $(LIB_DIR)/libbme68x.a

# Example executable
EXAMPLE_BIN = $(EXAMPLES_DIR)/basic_read

# Default target
all: directories $(STATIC_LIB) $(EXAMPLE_BIN)

# Create build directories
directories:
	@mkdir -p $(BUILD_DIR) $(LIB_DIR)

# Build static library
$(STATIC_LIB): $(BME68X_OBJ) $(LINUX_OBJ)
	@echo "Creating static library: $@"
	@ar rcs $@ $^
	@echo "Library created successfully!"

# Compile BME68X core
$(BME68X_OBJ): $(BME68X_SRC)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile Linux platform driver
$(LINUX_OBJ): $(LINUX_SRC)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile example
$(EXAMPLE_OBJ): $(EXAMPLE_SRC)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Link example
$(EXAMPLE_BIN): $(EXAMPLE_OBJ) $(STATIC_LIB)
	@echo "Linking: $@"
	@$(CC) $(EXAMPLE_OBJ) $(STATIC_LIB) $(LDFLAGS) -o $@
	@echo "Example built successfully!"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR) $(LIB_DIR)
	@rm -f $(EXAMPLE_BIN)
	@echo "Clean complete!"

# Install library (optional)
install: $(STATIC_LIB)
	@echo "Installing library to /usr/local/lib..."
	@sudo cp $(STATIC_LIB) /usr/local/lib/
	@echo "Installing headers to /usr/local/include/bme68x..."
	@sudo mkdir -p /usr/local/include/bme68x
	@sudo cp $(BME68X_DIR)/*.h /usr/local/include/bme68x/
	@sudo cp $(SRC_DIR)/*.h /usr/local/include/bme68x/
	@echo "Installation complete!"

# Uninstall library
uninstall:
	@echo "Uninstalling library..."
	@sudo rm -f /usr/local/lib/libbme68x.a
	@sudo rm -rf /usr/local/include/bme68x
	@echo "Uninstall complete!"

# Help
help:
	@echo "BME688 Linux Library - Makefile"
	@echo "================================"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build library and examples (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install library to /usr/local"
	@echo "  uninstall  - Remove installed library"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make           # Build everything"
	@echo "  make clean     # Clean build"
	@echo "  sudo make install   # Install system-wide"

.PHONY: all directories clean install uninstall help
