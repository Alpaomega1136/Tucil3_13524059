#include "Board.hpp"
#include "graph.hpp"
#include "pathFinding.hpp"

#include <fstream>
#include <iostream>
#include <string>

namespace {
bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string resolveInputPath(const std::string& path) {
    if (fileExists(path)) {
        return path;
    }

    std::string testPath = "test/" + path;
    if (fileExists(testPath)) {
        return testPath;
    }

    return path;
}

std::string boolText(bool value) {
    if (value) {
        return "yes";
    }
    return "no";
}

std::string readInputPath(int argc, char* argv[]) {
    if (argc >= 2) {
        return argv[1];
    }

    std::string pathFile;
    std::cout << "Masukkan file input: ";
    std::cin >> pathFile;
    return pathFile;
}

void printPathFindingResult(const std::string& label,
                            const PathFinding& pathFinding) {
    if (pathFinding.getPath().empty()) {
        std::cout << label << " path: tidak ditemukan\n";
        std::cout << label << " cost: -\n";
        std::cout << label << " iterations: "
                  << pathFinding.getTotalIterations() << '\n';
        return;
    }

    std::cout << label << " path: ";
    for (char direction : pathFinding.getPath()) {
        std::cout << direction;
    }
    std::cout << '\n';
    std::cout << label << " cost: "
              << pathFinding.getTotalCost() << '\n';
    std::cout << label << " iterations: "
              << pathFinding.getTotalIterations() << '\n';
}
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        std::cout << "Penggunaan: " << argv[0] << " <file-input>\n";
        return 0;
    }

    try {
        Board board = readBoardFromFile(resolveInputPath(readInputPath(argc, argv)));

        Graph graph;
        graph.buildFromBoard(board);

        PathFinding ucs;
        ucs.UCS(graph);

        PathFinding gbfs;
        gbfs.GBFS(graph, HeuristicMode::ImportantSequence);

        PathFinding astar;
        astar.AStar(graph, HeuristicMode::ImportantSequence);

        std::cout << "Jumlah node simple graph: "
                  << graph.getNodeCount() << '\n';
        printPathFindingResult("UCS", ucs);
        printPathFindingResult("GBFS", gbfs);
        printPathFindingResult("A*", astar);

        const GraphNode* root = graph.getRoot();
        if (root != nullptr) {
            std::cout << "Root: Node " << root->nodeId
                      << " (" << root->x << ", " << root->y << ")"
                      << " grid=(" << root->row << ", " << root->col << ")\n";
        }

        for (const GraphNode* node : graph.getNodes()) {
            std::cout << "Node " << node->nodeId
                      << " (" << node->x << ", " << node->y << ")"
                      << " grid=(" << node->row << ", " << node->col << ")"
                      << " type=" << node->type
                      << " branch=" << boolText(node->canBranch) << '\n';

            std::cout << "  Prediction: ";
            for (const PredictionCost& prediction : node->predictionCosts) {
                std::cout << prediction.target << "=" << prediction.cost << " ";
            }
            std::cout << '\n';
            std::cout << "  Heuristic nextNumber=0: important="
                      << graph.getHeuristicCost(node,
                                                0,
                                                HeuristicMode::ImportantSequence)
                      << " finishOnly="
                      << graph.getHeuristicCost(node,
                                                0,
                                                HeuristicMode::FinishOnly)
                      << '\n';

            for (const Edge& edge : node->neighbors) {
                std::cout << "  " << edge.direction
                          << " -> Node " << edge.neighborId
                          << " cost=" << edge.costEdge
                          << " h=" << edge.costPrediction
                          << " stop=" << boolText(edge.canStopAtNeighbor)
                          << '\n';
                std::cout << "    passedImportant: ";
                for (const PassedTile& tile : edge.passedImportant) {
                    std::cout << tile.type << "@(" << tile.row << ","
                              << tile.col << ") ";
                }
                std::cout << '\n';
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
