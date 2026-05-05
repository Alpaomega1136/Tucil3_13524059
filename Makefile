CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude -pthread

TARGET := bin/tucil3
BUILD_DIR := bin
SRC := $(filter-out src/mainGUI.cpp,$(wildcard src/*.cpp))
OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

.PHONY: all CLI run clean rebuild

all: CLI

CLI: $(TARGET)

$(TARGET): $(OBJ) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: CLI
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all
