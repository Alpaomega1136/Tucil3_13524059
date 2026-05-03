#include "Board.hpp"
#include "graph.hpp"
#include "pathFinding.hpp"

#include "raylib.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
const std::string INPUT_FOLDER = "test/input/";
const std::string OUTPUT_FOLDER = "test/output/";

enum class AlgorithmChoice {
    UCS,
    GBFS,
    AStar
};

struct GuiState {
    std::string inputPath = "test.txt";
    std::string message = "Masukkan file input, pilih algoritma, lalu tekan Solve.";
    AlgorithmChoice algorithm = AlgorithmChoice::AStar;
    HeuristicMode heuristic = HeuristicMode::FinishOnly;
    Board board;
    std::vector<char> path;
    std::vector<Position> positions;
    int totalCost = 0;
    int totalIterations = 0;
    int currentStep = 0;
    long long elapsedMs = 0;
    bool hasSolution = false;
};

std::filesystem::path getProjectRoot() {
    std::filesystem::path currentPath = std::filesystem::current_path();
    while (!currentPath.empty()) {
        if (currentPath.filename() == "Tucil3_13524059") {
            return currentPath;
        }
        if (std::filesystem::exists(currentPath / "Makefile") &&
            std::filesystem::exists(currentPath / "src") &&
            std::filesystem::exists(currentPath / "test")) {
            return currentPath;
        }
        if (currentPath == currentPath.root_path()) {
            break;
        }
        currentPath = currentPath.parent_path();
    }

    return std::filesystem::current_path();
}

std::string getProjectPath(const std::string& path) {
    std::filesystem::path filePath(path);
    if (filePath.is_absolute()) {
        return filePath.string();
    }

    return (getProjectRoot() / filePath).lexically_normal().string();
}

bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string resolveInputPath(const std::string& path) {
    std::string projectPath = getProjectPath(path);
    if (fileExists(projectPath)) {
        return projectPath;
    }

    std::string inputPath = getProjectPath(INPUT_FOLDER + path);
    if (fileExists(inputPath)) {
        return inputPath;
    }

    std::string testPath = getProjectPath("test/" + path);
    if (fileExists(testPath)) {
        return testPath;
    }

    return projectPath;
}

std::string directionsToString(const std::vector<char>& path) {
    std::string result;
    for (char direction : path) {
        result.push_back(direction);
    }
    return result;
}

std::string getOutputPath(const std::string& inputPath) {
    std::filesystem::path path(inputPath);
    std::string stem = path.stem().string();
    if (stem.empty()) {
        stem = "solusi";
    }

    return OUTPUT_FOLDER + stem + "_solution.txt";
}

void writeBoardWithActor(std::ostream& output, const Board& board, Position actor) {
    for (int row = 0; row < board.getRowCount(); ++row) {
        for (int col = 0; col < board.getColCount(); ++col) {
            if (row == actor.row && col == actor.col) {
                output << 'Z';
                continue;
            }

            char type = board.getType(row, col);
            if (type == 'Z') {
                output << '*';
            } else {
                output << type;
            }
        }
        output << '\n';
    }
}

void saveSolution(const GuiState& state) {
    std::string outputPath = getProjectPath(getOutputPath(state.inputPath));
    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());
    std::ofstream output(outputPath);
    if (!output) {
        throw std::runtime_error("File solusi tidak dapat dibuat.");
    }

    output << "Solusi Yang Ditemukan : " << directionsToString(state.path) << '\n';
    output << "Cost dari Solusi : " << state.totalCost << '\n';
    if (!state.positions.empty()) {
        output << "Initial\n";
        writeBoardWithActor(output, state.board, state.positions.front());
        for (std::size_t i = 0; i < state.path.size(); ++i) {
            output << "Step " << i + 1 << " : " << state.path[i] << '\n';
            writeBoardWithActor(output, state.board, state.positions[i + 1]);
        }
    }
    output << ">> Waktu eksekusi: " << state.elapsedMs << " ms\n";
    output << ">> Banyak iterasi yang dilakukan: " << state.totalIterations << " iterasi\n";
}

void solve(GuiState& state) {
    std::string resolvedInputPath = resolveInputPath(state.inputPath);
    state.board = readBoardFromFile(resolvedInputPath);

    Graph graph;
    graph.buildFromBoard(state.board);

    PathFinding pathFinding;
    auto startTime = std::chrono::steady_clock::now();
    if (state.algorithm == AlgorithmChoice::UCS) {
        pathFinding.UCS(graph);
    } else if (state.algorithm == AlgorithmChoice::GBFS) {
        pathFinding.GBFS(graph, state.heuristic);
    } else {
        pathFinding.AStar(graph, state.heuristic);
    }
    auto endTime = std::chrono::steady_clock::now();

    state.inputPath = resolvedInputPath;
    state.elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    state.path = pathFinding.getPath();
    state.positions = pathFinding.getPathPositions();
    state.totalCost = pathFinding.getTotalCost();
    state.totalIterations = pathFinding.getTotalIterations();
    state.currentStep = 0;
    state.hasSolution = !state.path.empty();
    if (state.hasSolution) {
        state.message = "Solusi ditemukan: " + directionsToString(state.path);
    } else {
        state.message = "Solusi tidak ditemukan.";
    }
}

void drawRoundedLines(Rectangle rect, float roundness, int segments, float lineThick, Color color);

bool drawButton(Rectangle rect, const std::string& text, bool selected = false) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    Color fill = selected ? Color{55, 102, 166, 255} : Color{236, 239, 244, 255};
    if (hovered) {
        fill = selected ? Color{47, 88, 143, 255} : Color{225, 230, 238, 255};
    }

    DrawRectangleRounded(rect, 0.14f, 8, fill);
    drawRoundedLines(rect, 0.14f, 8, 1.5f, selected ? Color{29, 61, 108, 255} : Color{132, 142, 156, 255});

    int fontSize = 18;
    int textWidth = MeasureText(text.c_str(), fontSize);
    Color textColor = selected ? RAYWHITE : Color{31, 41, 55, 255};
    DrawText(text.c_str(), static_cast<int>(rect.x + (rect.width - textWidth) / 2),
             static_cast<int>(rect.y + (rect.height - fontSize) / 2), fontSize, textColor);

    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void drawRoundedLines(Rectangle rect, float roundness, int segments, float lineThick, Color color) {
#if defined(RAYLIB_VERSION_MAJOR) && defined(RAYLIB_VERSION_MINOR) && \
    ((RAYLIB_VERSION_MAJOR > 5) || (RAYLIB_VERSION_MAJOR == 5 && RAYLIB_VERSION_MINOR >= 5))
    DrawRectangleRoundedLinesEx(rect, roundness, segments, lineThick, color);
#else
    DrawRectangleRoundedLines(rect, roundness, segments, lineThick, color);
#endif
}

void drawInputBox(Rectangle rect, std::string& value, bool& active) {
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        active = CheckCollisionPointRec(mouse, rect);
    }

    if (active) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126 && value.size() < 160) {
                value.push_back(static_cast<char>(key));
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !value.empty()) {
            value.pop_back();
        }
    }

    DrawRectangleRounded(rect, 0.08f, 8, RAYWHITE);
    drawRoundedLines(rect, 0.08f, 8, 1.5f, active ? Color{55, 102, 166, 255} : Color{132, 142, 156, 255});
    DrawText(value.c_str(), static_cast<int>(rect.x + 12), static_cast<int>(rect.y + 12), 18, Color{31, 41, 55, 255});
}

Color colorForTile(char type) {
    if (type == 'X') {
        return Color{61, 67, 78, 255};
    }
    if (type == 'L') {
        return Color{195, 72, 72, 255};
    }
    if (type == 'O') {
        return Color{67, 160, 102, 255};
    }
    if (type >= '0' && type <= '9') {
        return Color{236, 190, 85, 255};
    }
    return Color{232, 236, 242, 255};
}

void drawBoard(const GuiState& state, Rectangle area) {
    if (state.positions.empty()) {
        DrawText("Belum ada solusi untuk divisualisasikan.", static_cast<int>(area.x), static_cast<int>(area.y), 20, DARKGRAY);
        return;
    }

    int rows = state.board.getRowCount();
    int cols = state.board.getColCount();
    float cellSize = std::min(area.width / static_cast<float>(cols), area.height / static_cast<float>(rows));
    cellSize = std::min(cellSize, 54.0f);
    float boardWidth = cellSize * cols;
    float boardHeight = cellSize * rows;
    float startX = area.x + (area.width - boardWidth) / 2.0f;
    float startY = area.y + (area.height - boardHeight) / 2.0f;
    Position actor = state.positions[state.currentStep];

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            Rectangle cell{startX + col * cellSize, startY + row * cellSize, cellSize - 2, cellSize - 2};
            char type = state.board.getType(row, col);
            DrawRectangleRounded(cell, 0.08f, 4, colorForTile(type));
            drawRoundedLines(cell, 0.08f, 4, 1.0f, Color{128, 137, 150, 255});

            char shown = type;
            if (type == 'Z') {
                shown = '*';
            }
            if (actor.row == row && actor.col == col) {
                shown = 'Z';
                DrawRectangleRounded(cell, 0.08f, 4, Color{65, 105, 225, 255});
            }

            std::string label(1, shown);
            int fontSize = static_cast<int>(std::max(18.0f, cellSize * 0.42f));
            int textWidth = MeasureText(label.c_str(), fontSize);
            DrawText(label.c_str(), static_cast<int>(cell.x + (cell.width - textWidth) / 2),
                     static_cast<int>(cell.y + (cell.height - fontSize) / 2), fontSize, RAYWHITE);
        }
    }
}

std::string heuristicName(HeuristicMode mode) {
    if (mode == HeuristicMode::FinishOnly) {
        return "H1 FinishOnly";
    }
    if (mode == HeuristicMode::ImportantSequence) {
        return "H2 ImportantSequence";
    }
    return "H3 OrderedSequence";
}
}

int main() {
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(1180, 760, "Tucil3 Ice Sliding Puzzle Solver");
    SetTargetFPS(60);

    GuiState state;
    bool inputActive = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{246, 248, 251, 255});

        DrawText("Ice Sliding Puzzle Solver", 30, 24, 30, Color{31, 41, 55, 255});
        DrawText("Input file", 30, 78, 18, Color{75, 85, 99, 255});
        drawInputBox(Rectangle{120, 66, 360, 46}, state.inputPath, inputActive);

        if (drawButton(Rectangle{500, 66, 92, 46}, "UCS", state.algorithm == AlgorithmChoice::UCS)) {
            state.algorithm = AlgorithmChoice::UCS;
        }
        if (drawButton(Rectangle{602, 66, 92, 46}, "GBFS", state.algorithm == AlgorithmChoice::GBFS)) {
            state.algorithm = AlgorithmChoice::GBFS;
        }
        if (drawButton(Rectangle{704, 66, 92, 46}, "A*", state.algorithm == AlgorithmChoice::AStar)) {
            state.algorithm = AlgorithmChoice::AStar;
        }

        if (drawButton(Rectangle{820, 66, 92, 46}, "H1", state.heuristic == HeuristicMode::FinishOnly)) {
            state.heuristic = HeuristicMode::FinishOnly;
        }
        if (drawButton(Rectangle{922, 66, 92, 46}, "H2", state.heuristic == HeuristicMode::ImportantSequence)) {
            state.heuristic = HeuristicMode::ImportantSequence;
        }
        if (drawButton(Rectangle{1024, 66, 92, 46}, "H3", state.heuristic == HeuristicMode::OrderedSequence)) {
            state.heuristic = HeuristicMode::OrderedSequence;
        }

        if (drawButton(Rectangle{30, 128, 120, 42}, "Solve")) {
            try {
                solve(state);
            } catch (const std::exception& error) {
                state.hasSolution = false;
                state.positions.clear();
                state.path.clear();
                state.message = std::string("Error: ") + error.what();
            }
        }
        if (drawButton(Rectangle{160, 128, 120, 42}, "Save", false)) {
            try {
                if (state.hasSolution) {
                    saveSolution(state);
                    state.message = "Solusi disimpan pada " + getOutputPath(state.inputPath);
                }
            } catch (const std::exception& error) {
                state.message = std::string("Error: ") + error.what();
            }
        }
        if (drawButton(Rectangle{300, 128, 88, 42}, "Prev") && state.currentStep > 0) {
            --state.currentStep;
        }
        if (drawButton(Rectangle{398, 128, 88, 42}, "Next") && state.currentStep + 1 < static_cast<int>(state.positions.size())) {
            ++state.currentStep;
        }

        DrawText(state.message.c_str(), 510, 140, 18, Color{55, 65, 81, 255});
        DrawText(("Heuristic: " + heuristicName(state.heuristic)).c_str(), 30, 190, 18, Color{75, 85, 99, 255});

        std::string summary = "Path: " + directionsToString(state.path) +
                              "   Cost: " + std::to_string(state.totalCost) +
                              "   Iterasi: " + std::to_string(state.totalIterations) +
                              "   Waktu: " + std::to_string(state.elapsedMs) + " ms";
        DrawText(summary.c_str(), 30, 218, 18, Color{75, 85, 99, 255});

        std::string stepText = "Step " + std::to_string(state.currentStep) + " / " +
                               std::to_string(state.positions.empty() ? 0 : static_cast<int>(state.positions.size()) - 1);
        DrawText(stepText.c_str(), 30, 246, 18, Color{75, 85, 99, 255});

        drawBoard(state, Rectangle{30, 280, 1120, 440});
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
