CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude

TARGET := bin/tucil3
GUI_TARGET := bin/tucil3-gui
BUILD_DIR := bin
COMMON_SRC := $(filter-out src/main.cpp src/mainGUI.cpp,$(wildcard src/*.cpp))
CLI_SRC := src/main.cpp
GUI_SRC := src/mainGUI.cpp
COMMON_OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(COMMON_SRC))
CLI_OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(CLI_SRC))
GUI_OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(GUI_SRC))
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS = $(shell pkg-config --libs raylib)

.PHONY: all CLI GUI run run-gui clean rebuild

all: CLI

CLI: $(TARGET)

GUI: $(GUI_TARGET)

$(TARGET): $(COMMON_OBJ) $(CLI_OBJ) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@

$(GUI_TARGET): $(COMMON_OBJ) $(GUI_OBJ) | bin
	$(CXX) $(CXXFLAGS) $(RAYLIB_CFLAGS) $^ -o $@ $(RAYLIB_LIBS)

$(GUI_OBJ): $(GUI_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(RAYLIB_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: CLI
	./$(TARGET)

run-gui: GUI
	./$(GUI_TARGET)

clean:
	rm -f $(COMMON_OBJ) $(CLI_OBJ) $(GUI_OBJ) $(TARGET) $(GUI_TARGET)

rebuild: clean all
