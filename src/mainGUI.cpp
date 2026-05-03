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
const int WINDOW_WIDTH = 2560;
const int WINDOW_HEIGHT = 1440;
const float UI_SCALE = 1.25f;
const Vector2 UI_OFFSET{80.0f, 45.0f};

Font guiFont;
bool hasGuiFont = false;
Camera2D guiCamera;

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
    if (state.inputPath.empty()) {
        throw std::runtime_error("Nama file input tidak boleh kosong.");
    }

    std::string resolvedInputPath = resolveInputPath(state.inputPath);
    if (!fileExists(resolvedInputPath)) {
        throw std::runtime_error("File input '" + state.inputPath + "' tidak ditemukan.");
    }

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

std::vector<std::string> getPoppinsFontPaths() {
    return {
        getProjectPath("assets/fonts/Poppins-Regular.ttf"),
        getProjectPath("assets/font/Poppins-Regular.ttf"),
        getProjectPath("font/Poppins-Regular.ttf"),
        getProjectPath("Poppins-Regular.ttf"),
        "/usr/share/fonts/truetype/poppins/Poppins-Regular.ttf",
        "/usr/share/fonts/opentype/poppins/Poppins-Regular.otf"
    };
}

void loadGuiFont() {
    for (const std::string& fontPath : getPoppinsFontPaths()) {
        if (!std::filesystem::exists(fontPath)) {
            continue;
        }

        guiFont = LoadFontEx(fontPath.c_str(), 96, nullptr, 0);
        if (guiFont.texture.id != 0) {
            SetTextureFilter(guiFont.texture, TEXTURE_FILTER_BILINEAR);
            hasGuiFont = true;
            return;
        }
    }
}

float measureTextWidth(const std::string& text, int fontSize) {
    if (hasGuiFont) {
        return MeasureTextEx(guiFont, text.c_str(), static_cast<float>(fontSize), 1.0f).x;
    }

    return static_cast<float>(MeasureText(text.c_str(), fontSize));
}

void drawGuiText(const std::string& text, int x, int y, int fontSize, Color color) {
    if (hasGuiFont) {
        DrawTextEx(guiFont, text.c_str(), Vector2{static_cast<float>(x), static_cast<float>(y)}, static_cast<float>(fontSize), 1.0f, color);
        return;
    }

    DrawText(text.c_str(), x, y, fontSize, color);
}

Vector2 getGuiMousePosition() {
    return GetScreenToWorld2D(GetMousePosition(), guiCamera);
}

bool drawButton(Rectangle rect, const std::string& text, bool selected = false) {
    Vector2 mouse = getGuiMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    Color fill = selected ? Color{70, 130, 180, 255} : Color{80, 80, 80, 255};
    if (hovered) {
        fill = selected ? Color{60, 110, 160, 255} : Color{100, 100, 100, 255};
    }

    DrawRectangleRounded(rect, 0.14f, 8, fill);
    drawRoundedLines(rect, 0.14f, 8, 1.5f, selected ? Color{40, 90, 140, 255} : Color{120, 120, 120, 255});

    int fontSize = 24;
    float textWidth = measureTextWidth(text, fontSize);
    Color textColor = RAYWHITE;
    drawGuiText(text, static_cast<int>(rect.x + (rect.width - textWidth) / 2),
                static_cast<int>(rect.y + (rect.height - fontSize) / 2), fontSize, textColor);

    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void drawPanel(Rectangle rect, const std::string& title = "") {
    DrawRectangleRounded(rect, 0.035f, 12, Color{60, 60, 60, 255});
    drawRoundedLines(rect, 0.035f, 12, 1.2f, Color{100, 100, 100, 255});
    if (!title.empty()) {
        drawGuiText(title, static_cast<int>(rect.x + 24), static_cast<int>(rect.y + 22), 32, RAYWHITE);
    }
}

void drawMetric(Rectangle rect, const std::string& label, const std::string& value) {
    DrawRectangleRounded(rect, 0.08f, 8, Color{75, 75, 75, 255});
    drawRoundedLines(rect, 0.08f, 8, 1.0f, Color{110, 110, 110, 255});
    drawGuiText(label, static_cast<int>(rect.x + 16), static_cast<int>(rect.y + 10), 20, Color{200, 200, 200, 255});
    drawGuiText(value, static_cast<int>(rect.x + 16), static_cast<int>(rect.y + 40), 30, RAYWHITE);
}

void drawRoundedLines(Rectangle rect, float roundness, int segments, float lineThick, Color color) {
#if defined(RAYLIB_VERSION_MAJOR) && defined(RAYLIB_VERSION_MINOR) && \
    ((RAYLIB_VERSION_MAJOR > 5) || (RAYLIB_VERSION_MAJOR == 5 && RAYLIB_VERSION_MINOR >= 5))
    DrawRectangleRoundedLinesEx(rect, roundness, segments, lineThick, color);
#else
    DrawRectangleRoundedLines(rect, roundness, segments, lineThick, color);
#endif
}

char keyToInputChar(int key) {
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (key >= KEY_A && key <= KEY_Z) {
        char base = shift ? 'A' : 'a';
        return static_cast<char>(base + (key - KEY_A));
    }
    if (key >= KEY_ZERO && key <= KEY_NINE) {
        return static_cast<char>('0' + (key - KEY_ZERO));
    }
    if (key >= KEY_KP_0 && key <= KEY_KP_9) {
        return static_cast<char>('0' + (key - KEY_KP_0));
    }

    if (key == KEY_PERIOD || key == KEY_KP_DECIMAL) {
        return '.';
    }
    if (key == KEY_MINUS || key == KEY_KP_SUBTRACT) {
        return shift ? '_' : '-';
    }
    if (key == KEY_SLASH || key == KEY_KP_DIVIDE) {
        return '/';
    }
    if (key == KEY_BACKSLASH) {
        return '\\';
    }
    if (key == KEY_SPACE) {
        return ' ';
    }

    return '\0';
}

void drawInputBox(Rectangle rect, std::string& value, bool& active) {
    Vector2 mouse = getGuiMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        active = CheckCollisionPointRec(mouse, rect);
    }

    if (active) {
        bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        bool handledShortcut = false;
        if (ctrl && IsKeyPressed(KEY_A)) {
            value.clear();
            handledShortcut = true;
        }
        if (ctrl && IsKeyPressed(KEY_V)) {
            const char* clipboard = GetClipboardText();
            if (clipboard != nullptr) {
                value += clipboard;
            }
            handledShortcut = true;
        }

        if (!handledShortcut && !ctrl) {
            int pressedKey = GetKeyPressed();
            while (pressedKey > 0) {
                char character = keyToInputChar(pressedKey);
                if (character != '\0' && value.size() < 160) {
                    value.push_back(character);
                }
                pressedKey = GetKeyPressed();
            }
        }

        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && !value.empty()) {
            value.pop_back();
        }
    }

    DrawRectangleRounded(rect, 0.08f, 8, Color{50, 50, 50, 255});
    drawRoundedLines(rect, 0.08f, 8, 1.5f, active ? Color{100, 150, 255, 255} : Color{100, 100, 100, 255});

    std::string shown = value;
    int fontSize = 26;
    while (measureTextWidth(shown, fontSize) > rect.width - 28 && shown.size() > 3) {
        shown.erase(shown.begin());
        shown[0] = '.';
        shown[1] = '.';
        shown[2] = '.';
    }
    drawGuiText(shown, static_cast<int>(rect.x + 14), static_cast<int>(rect.y + 12), fontSize, RAYWHITE);
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
        drawGuiText("Belum ada solusi untuk divisualisasikan.", static_cast<int>(area.x), static_cast<int>(area.y), 28, Color{150, 150, 150, 255});
        return;
    }

    int rows = state.board.getRowCount();
    int cols = state.board.getColCount();
    float cellSize = std::min(area.width / static_cast<float>(cols), area.height / static_cast<float>(rows));
    cellSize = std::min(cellSize, 112.0f);
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
            int fontSize = static_cast<int>(std::max(32.0f, cellSize * 0.52f));
            float textWidth = measureTextWidth(label, fontSize);
            drawGuiText(label, static_cast<int>(cell.x + (cell.width - textWidth) / 2),
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
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tucil3 Ice Sliding Puzzle Solver");
    SetTargetFPS(60);
    loadGuiFont();
    guiCamera = Camera2D{UI_OFFSET, Vector2{0.0f, 0.0f}, 0.0f, UI_SCALE};

    GuiState state;
    bool inputActive = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{45, 45, 45, 255});
        BeginMode2D(guiCamera);

        drawGuiText("Ice Sliding Puzzle Solver", 44, 30, 60, RAYWHITE);
        drawGuiText("Tucil3_13524059", 44, 102, 30, Color{200, 200, 200, 255});

        Rectangle controlPanel{36, 126, 430, 930};
        Rectangle boardPanel{498, 126, 1386, 842};
        drawPanel(controlPanel, "Kontrol");
        drawPanel(boardPanel, "Visualisasi Board");

        drawGuiText("Input file", 64, 190, 28, Color{220, 220, 220, 255});
        drawInputBox(Rectangle{64, 226, 374, 62}, state.inputPath, inputActive);

        drawGuiText("Ketik nama file atau path input", 64, 306, 22, Color{180, 180, 180, 255});

        drawGuiText("Algoritma", 64, 380, 28, Color{220, 220, 220, 255});
        if (drawButton(Rectangle{64, 418, 112, 60}, "UCS", state.algorithm == AlgorithmChoice::UCS)) {
            state.algorithm = AlgorithmChoice::UCS;
        }
        if (drawButton(Rectangle{188, 418, 112, 60}, "GBFS", state.algorithm == AlgorithmChoice::GBFS)) {
            state.algorithm = AlgorithmChoice::GBFS;
        }
        if (drawButton(Rectangle{312, 418, 112, 60}, "A*", state.algorithm == AlgorithmChoice::AStar)) {
            state.algorithm = AlgorithmChoice::AStar;
        }

        drawGuiText("Heuristik", 64, 510, 28, Color{220, 220, 220, 255});
        if (drawButton(Rectangle{64, 548, 112, 60}, "H1", state.heuristic == HeuristicMode::FinishOnly)) {
            state.heuristic = HeuristicMode::FinishOnly;
        }
        if (drawButton(Rectangle{188, 548, 112, 60}, "H2", state.heuristic == HeuristicMode::ImportantSequence)) {
            state.heuristic = HeuristicMode::ImportantSequence;
        }
        if (drawButton(Rectangle{312, 548, 112, 60}, "H3", state.heuristic == HeuristicMode::OrderedSequence)) {
            state.heuristic = HeuristicMode::OrderedSequence;
        }

        drawGuiText(heuristicName(state.heuristic), 64, 622, 24, Color{180, 180, 180, 255});

        bool solvePressed = drawButton(Rectangle{64, 682, 174, 66}, "Solve") || (inputActive && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)));
        if (solvePressed) {
            try {
                solve(state);
            } catch (const std::exception& error) {
                state.hasSolution = false;
                state.positions.clear();
                state.path.clear();
                state.message = std::string("Error: ") + error.what();
            }
        }
        if (drawButton(Rectangle{250, 682, 174, 66}, "Save", false)) {
            try {
                if (state.hasSolution) {
                    saveSolution(state);
                    state.message = "Solusi disimpan pada " + getOutputPath(state.inputPath);
                } else {
                    state.message = "Error: Tidak ada solusi untuk disimpan.";
                }
            } catch (const std::exception& error) {
                state.message = std::string("Error: ") + error.what();
            }
        }

        drawGuiText("Playback", 64, 790, 28, Color{220, 220, 220, 255});
        bool prevPressed = drawButton(Rectangle{64, 828, 174, 62}, "Prev") || (!inputActive && IsKeyPressed(KEY_LEFT));
        if (prevPressed && state.currentStep > 0) {
            --state.currentStep;
        }
        bool nextPressed = drawButton(Rectangle{250, 828, 174, 62}, "Next") || (!inputActive && IsKeyPressed(KEY_RIGHT));
        if (nextPressed && state.currentStep + 1 < static_cast<int>(state.positions.size())) {
            ++state.currentStep;
        }

        std::string stepText = "Step " + std::to_string(state.currentStep) + " / " +
                               std::to_string(state.positions.empty() ? 0 : static_cast<int>(state.positions.size()) - 1);
        drawGuiText(stepText, 64, 906, 34, RAYWHITE);

        drawMetric(Rectangle{536, 200, 266, 94}, "Path", state.path.empty() ? "-" : directionsToString(state.path));
        drawMetric(Rectangle{824, 200, 178, 94}, "Cost", std::to_string(state.totalCost));
        drawMetric(Rectangle{1024, 200, 196, 94}, "Iterasi", std::to_string(state.totalIterations));
        drawMetric(Rectangle{1242, 200, 196, 94}, "Waktu", std::to_string(state.elapsedMs) + " ms");
        drawMetric(Rectangle{1460, 200, 178, 94}, "Step", std::to_string(state.currentStep));

        drawBoard(state, Rectangle{536, 330, 1310, 480});

        Rectangle statusArea{536, 842, 1310, 94};
        DrawRectangleRounded(statusArea, 0.035f, 10, Color{58, 58, 58, 255});
        drawRoundedLines(statusArea, 0.035f, 10, 1.0f, Color{95, 95, 95, 255});
        drawGuiText("Status", 560, 860, 28, Color{220, 220, 220, 255});

        Color statusColor = (state.message.find("Error") != std::string::npos) ? Color{255, 100, 100, 255} : Color{180, 255, 180, 255};
        if (state.message.find("Solusi tidak ditemukan") != std::string::npos || 
            state.message == "Masukkan file input, pilih algoritma, lalu tekan Solve.") {
            statusColor = Color{200, 200, 200, 255};
        }
        drawGuiText(state.message, 560, 898, 24, statusColor);
        EndMode2D();
        EndDrawing();
    }

    if (hasGuiFont) {
        UnloadFont(guiFont);
    }
    CloseWindow();
    return 0;
}
