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
    if (argc < 2) {
        std::cout << "Penggunaan: " << argv[0] << " <file-input>\n";
        return 0;
    }

    try {
        Graph graph = readMapFromFile(argv[1]);
        SimpleGraph simpleGraph;
        simpleGraph.convertToSimpleGraph(graph);

        std::cout << "Jumlah node simple graph: "
                  << simpleGraph.getNodeCount() << '\n';

        const SimpleNode* root = simpleGraph.getRoot();
        if (root != nullptr) {
            std::cout << "Root: Node " << root->nodeId
                      << " (" << root->x << ", " << root->y << ")"
                      << " grid=(" << root->row << ", " << root->col << ")\n";
        }

        for (const SimpleNode* node : simpleGraph.getNodes()) {
            std::cout << "Node " << node->nodeId
                      << " (" << node->x << ", " << node->y << ")"
                      << " grid=(" << node->row << ", " << node->col << ")"
                      << " type=" << node->type
                      << " branch=" << boolText(node->canBranch) << '\n';

            for (const informationEdge& edge : node->neighbors) {
                std::cout << "  " << edge.direction
                          << " -> Node " << edge.neighborId
                          << " cost=" << edge.costEdge
                          << " h=" << edge.costPrediction
                          << " stop=" << boolText(edge.canStopAtNeighbor)
                          << '\n';
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
