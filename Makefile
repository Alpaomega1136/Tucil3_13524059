CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude

TARGET := bin/tucil3
BUILD_DIR := build
SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

.PHONY: all run clean rebuild

all: $(TARGET)

$(TARGET): $(OBJ) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR) bin:
	mkdir -p $@

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

rebuild: clean all
