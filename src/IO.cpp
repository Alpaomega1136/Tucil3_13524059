#include "IO.hpp"

#include <filesystem>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace {
const std::string INPUT_FOLDER = "test/input/";
const std::string OUTPUT_FOLDER = "test/output/";

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

std::string safeFilePart(const std::string& value) {
    std::string result;
    for (char character : value) {
        if ((character >= 'A' && character <= 'Z') ||
            (character >= 'a' && character <= 'z') ||
            (character >= '0' && character <= '9')) {
            result.push_back(character);
        } else if (character == '*') {
            result += "Star";
        } else if (character == '+' || character == '-' || character == '_') {
            result.push_back('_');
        }
    }

    if (result.empty()) {
        return "None";
    }

    return result;
}
}

bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string getProjectPath(const std::string& path) {
    std::filesystem::path filePath(path);
    if (filePath.is_absolute()) {
        return filePath.string();
    }

    return (getProjectRoot() / filePath).lexically_normal().string();
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

std::string getOutputPath(const std::string& inputPath) {
    std::filesystem::path path(inputPath);
    std::string stem = path.stem().string();
    if (stem.empty()) {
        stem = "solusi";
    }

    return OUTPUT_FOLDER + stem + "_solution.txt";
}

std::string getOutputPath(const std::string& inputPath, const std::string& algorithm, const std::string& heuristic) {
    std::filesystem::path path(inputPath);
    std::string stem = path.stem().string();
    if (stem.empty()) {
        stem = "solusi";
    }

    return OUTPUT_FOLDER + stem + "_solution_" + safeFilePart(algorithm) + "_" + safeFilePart(heuristic) + ".txt";
}

std::string directionsToString(const std::vector<char>& path) {
    std::string result;
    for (char direction : path) {
        result.push_back(direction);
    }
    return result;
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

void writeSolutionSteps(std::ostream& output, const Board& board, const std::vector<char>& path, const std::vector<Position>& positions) {
    if (positions.empty()) {
        return;
    }

    output << "Initial\n";
    writeBoardWithActor(output, board, positions.front());

    for (std::size_t i = 0; i < path.size(); ++i) {
        output << "Step " << i + 1 << " : " << path[i] << '\n';
        writeBoardWithActor(output, board, positions[i + 1]);
    }
}

void saveSolution(const Board& board, const std::vector<char>& path, const std::vector<Position>& positions, int totalCost,
                int totalIterations, long long elapsedMs, const std::string& outputPath) {
    saveSolution(board, path, positions, totalCost, totalIterations, elapsedMs, outputPath, "-", "-");
}

void saveSolution(const Board& board, const std::vector<char>& path, const std::vector<Position>& positions, int totalCost,
                int totalIterations, long long elapsedMs, const std::string& outputPath, const std::string& algorithm,
                const std::string& heuristic) {
    std::string projectOutputPath = getProjectPath(outputPath);
    std::filesystem::create_directories(std::filesystem::path(projectOutputPath).parent_path());
    std::ofstream output(projectOutputPath);
    if (!output) {
        throw std::runtime_error("File solusi tidak dapat dibuat.");
    }

    output << "Algoritma Pathfinding : " << algorithm << '\n';
    output << "Tipe Heuristik : " << heuristic << '\n';
    output << "Solusi Yang Ditemukan : " << directionsToString(path) << '\n';
    output << "Cost dari Solusi : " << totalCost << '\n';
    writeSolutionSteps(output, board, path, positions);
    output << ">> Waktu eksekusi: " << elapsedMs << " ms\n";
    output << ">> Banyak iterasi yang dilakukan: " << totalIterations << " iterasi\n";
}
