#include "Board.hpp"
#include "graph.hpp"
#include "pathFinding.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

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

    return path;
}

std::string getOutputPath(const std::string& inputPath) {
    std::filesystem::path path(inputPath);
    std::string stem = path.stem().string();
    if (stem.empty()) {
        stem = "solusi";
    }

    return OUTPUT_FOLDER + stem + "_solution.txt";
}

std::string directionsToString(const std::vector<char>& path) {
    std::string result;
    for (char direction : path) {
        result.push_back(direction);
    }
    return result;
}

std::string readInputPath(int argc, char* argv[]) {
    if (argc >= 2) {
        return resolveInputPath(argv[1]);
    }

    std::string pathFile;
    std::cout << ">> Masukan file input :\n";
    std::cin >> pathFile;
    return resolveInputPath(pathFile);
}

std::string readAlgorithm() {
    std::string algorithm;
    std::cout << ">> Algoritma apa yang anda pilih? (UCS/GBFS/A*)\n";
    std::cin >> algorithm;
    return algorithm;
}

HeuristicMode readHeuristicMode() {
    std::string heuristic;
    std::cout << ">> Heuristic apa yang anda pilih? (H1/H2/H3)\n";
    std::cin >> heuristic;

    if (heuristic == "H1") {
        return HeuristicMode::FinishOnly;
    }
    if (heuristic == "H2") {
        return HeuristicMode::ImportantSequence;
    }
    if (heuristic == "H3") {
        return HeuristicMode::OrderedSequence;
    }

    throw std::runtime_error("Heuristic harus H1, H2, atau H3.");
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

void printBoardWithActor(const Board& board, Position actor) {
    writeBoardWithActor(std::cout, board, actor);
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

void printSolutionSteps(const Board& board, const std::vector<char>& path, const std::vector<Position>& positions) {
    writeSolutionSteps(std::cout, board, path, positions);
}

void runPlayback(const Board& board, const std::vector<Position>& positions, std::size_t maxStep) {
    if (positions.empty()) {
        return;
    }

    std::string answer;
    std::cout << ">> Apakah Anda ingin melakukan playback? (Ya/Tidak) :\n";
    std::cin >> answer;
    if (answer != "Ya") {
        return;
    }

    std::size_t step = 0;
    std::cout << ">> Pada step berapa anda ingin melakukan playback :\n";
    std::cin >> step;
    if (step > maxStep) {
        step = maxStep;
    }

    std::cout << "Playback Step " << step << '\n';
    printBoardWithActor(board, positions[step]);
}

void saveSolution(const Board& board, const std::vector<char>& path, const std::vector<Position>& positions, int totalCost,
                  int totalIterations,long long elapsedMs, const std::string& outputPath) {
    std::string projectOutputPath = getProjectPath(outputPath);
    std::filesystem::create_directories(std::filesystem::path(projectOutputPath).parent_path());
    std::ofstream output(projectOutputPath);
    if (!output) {
        throw std::runtime_error("File solusi tidak dapat dibuat.");
    }

    output << "Solusi Yang Ditemukan : " << directionsToString(path) << '\n';
    output << "Cost dari Solusi : " << totalCost << '\n';
    writeSolutionSteps(output, board, path, positions);
    output << ">> Waktu eksekusi: " << elapsedMs << " ms\n";
    output << ">> Banyak iterasi yang dilakukan: " << totalIterations << " iterasi\n";
}

void askToSaveSolution(const Board& board, const std::vector<char>& path, const std::vector<Position>& positions, int totalCost,
                       int totalIterations, long long elapsedMs, const std::string& outputPath) {
    std::string answer;
    std::cout << ">> Apakah Anda ingin menyimpan solusi? (Ya/Tidak) :\n";
    std::cin >> answer;
    if (answer != "Ya") {
        return;
    }

    saveSolution(board, path, positions, totalCost, totalIterations, elapsedMs, outputPath);
    std::cout << ">> Solusi disimpan pada " << outputPath << '\n';
}

void solveWithAlgorithm(const std::string& algorithm, HeuristicMode heuristicMode, const Graph& graph, PathFinding& pathFinding) {
    if (algorithm == "UCS") {
        pathFinding.UCS(graph);
    } else if (algorithm == "GBFS") {
        pathFinding.GBFS(graph, heuristicMode);
    } else {
        pathFinding.AStar(graph, heuristicMode);
    }
}
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        std::cout << "Penggunaan: " << argv[0] << " <file-input>\n";
        return 0;
    }

    try {
        std::string inputPath = readInputPath(argc, argv);
        Board board = readBoardFromFile(inputPath);
        Graph graph;
        graph.buildFromBoard(board);

        std::string algorithm = readAlgorithm();
        HeuristicMode heuristicMode = HeuristicMode::ImportantSequence;
        if (algorithm == "GBFS" || algorithm == "A*") {
            heuristicMode = readHeuristicMode();
        }

        if (algorithm != "UCS" && algorithm != "GBFS" && algorithm != "A*") {
            throw std::runtime_error("Algoritma harus UCS, GBFS, atau A*.");
        }

        PathFinding pathFinding;
        auto startTime = std::chrono::steady_clock::now();
        solveWithAlgorithm(algorithm, heuristicMode, graph, pathFinding);
        auto endTime = std::chrono::steady_clock::now();

        long long elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        std::vector<char> path = pathFinding.getPath();
        std::vector<Position> positions = pathFinding.getPathPositions();

        // Path tidak ditemukan
        if (path.empty()) {
            std::cout << "Solusi Tidak Ditemukan\n";
            std::cout << ">> Waktu eksekusi: " << elapsedMs << " ms\n";
            std::cout << ">> Banyak iterasi yang dilakukan: "
                      << pathFinding.getTotalIterations() << " iterasi\n";
            return 0;
        }

        // Path ditemukan
        std::cout << "Solusi Yang Ditemukan : " << directionsToString(path) << '\n';
        std::cout << "Cost dari Solusi : " << pathFinding.getTotalCost() << '\n';
        printSolutionSteps(board, path, positions);
        std::cout << ">> Waktu eksekusi: " << elapsedMs << " ms\n";
        std::cout << ">> Banyak iterasi yang dilakukan: "
                  << pathFinding.getTotalIterations() << " iterasi\n";
        runPlayback(board, positions, path.size());
        askToSaveSolution(board, path, positions, pathFinding.getTotalCost(), pathFinding.getTotalIterations(), elapsedMs, getOutputPath(inputPath));
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
