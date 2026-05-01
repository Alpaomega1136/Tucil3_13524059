#include "Board.hpp"
#include "graph.hpp"

#include <iostream>
#include <string>

namespace {
std::string boolText(bool value) {
    if (value) {
        return "yes";
    }
    return "no";
}
}

int main(int argc, char* argv[]) {
    std::string pathFile;
    if (argc >= 3) {
        std::cout << "Penggunaan: " << argv[0] << " <file-input>\n";
        return 0;
    } else if (argc < 2) {
        std::cout << "Masukkan file input: ";
        std::cin >> pathFile;
    }

    try {
        Board board;
        if (!pathFile.empty()) {
            board = readBoardFromFile(pathFile);
        } else {
            board = readBoardFromFile(argv[1]);
        }

        Graph graph;
        graph.buildFromBoard(board);

        std::cout << "Jumlah node simple graph: "
                  << graph.getNodeCount() << '\n';

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

        for (const GraphNode* node : graph.getNodes()) {
            std::cout << "Node " << node->nodeId << '\n';
            std::cout << "  Neighbors: ";
            for (const Edge& edge : node->neighbors) {
                std::cout << edge.neighborId << " ";
                std::cout << "(cost=" << edge.costEdge
                          << ", h=" << edge.costPrediction
                          << ", passed=" << edge.passedImportant.size()
                          << ") ";
            }
            std::cout << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
