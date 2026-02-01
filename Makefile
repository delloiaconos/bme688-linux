# Name of the final program
TARGET_NAME = app

# Extra C Sources
EXTRA_C_SRCS   += 
EXTRA_CPP_SRCS += ./app.cpp


# Compilers
CC  = gcc
CXX = g++

# Paths (modify to your setup)
INCLUDES = -I./inc -I./bosch-bme68x-library/src -I./bosch-bme68x-library/src/bme68x # headers
LIBDIRS  = -L./lib                												# libraries
SRC_DIRS = src bosch-bme68x-library/src bosch-bme68x-library/src/bme68x     # cartelle con file .c e .cpp

# Output directories
OUT_DIR  = out
OBJ_DIR  = $(OUT_DIR)/obj
DEP_DIR  = $(OUT_DIR)/dep

# Final binary path (goes to output/)
TARGET   = $(OUT_DIR)/$(TARGET_NAME)

# Flags
CFLAGS   = -O2 -Wall $(INCLUDES)
CXXFLAGS = -O2 -Wall $(INCLUDES)
LDFLAGS  = $(LIBDIRS) -lm     # -lm for math

# Find sources in SRC_DIRS
C_SRCS   = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
CPP_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))

# Add extra sources
C_SRCS   := $(C_SRCS)   $(EXTRA_C_SRCS)
CPP_SRCS := $(CPP_SRCS) $(EXTRA_CPP_SRCS)

# Map sources -> objects in output/obj/ (preserve subdir structure)
C_OBJS   = $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_SRCS))
CPP_OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(CPP_SRCS))
OBJS     = $(C_OBJS) $(CPP_OBJS)

# Map objects -> deps in output/dep/ (same structure)
DEPS     = $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$(OBJS))

# Default rule
all: $(TARGET)

# Ensure base output dirs exist
$(OUT_DIR) $(OBJ_DIR) $(DEP_DIR):
	mkdir -p $@

# Compile C sources -> output/obj/...
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR) $(DEP_DIR)
	@mkdir -p $(dir $@) $(dir $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$@))
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@ -MF $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$@)

# Compile C++ sources -> output/obj/...
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR) $(DEP_DIR)
	@mkdir -p $(dir $@) $(dir $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$@))
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@ -MF $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$@)

# Link -> output/app
$(TARGET): $(OBJS) | $(OUT_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Include dependency files if present
-include $(DEPS)

# Cleanup
clean:
	rm -rf $(OUT_DIR)