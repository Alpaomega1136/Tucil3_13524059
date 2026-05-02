#include "graph.hpp"

#include <cmath>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <utility>

namespace {
const int DELTA_ROW[] = {-1, 0, 1, 0};
const int DELTA_COL[] = {0, 1, 0, -1};
const char DIRECTIONS[] = {'U', 'R', 'D', 'L'};

char targetForNextNumber(int nextNumber, int importantNodeCount) {
    if (nextNumber >= 0 && nextNumber < importantNodeCount) {
        return static_cast<char>('0' + nextNumber);
    }

    return 'O';
}
}

Graph::~Graph() {
    clear();
}

void Graph::clear() {
    for (GraphNode* node : nodes) {
        delete node;
    }
    nodes.clear();
    root = nullptr;
    importantNodeCount = 0;
    importantTargets.clear();
    goal = Position{};
}

GraphNode* Graph::addNode(int row, int col) {
    GraphNode* node = new GraphNode;
    node->nodeId = static_cast<int>(nodes.size());
    node->row = row;
    node->col = col;
    node->x = col;
    node->y = -row;
    node->type = '*';
    node->canBranch = true;
    node->finishPredictionCost = 0;

    nodes.push_back(node);
    return node;
}

GraphNode* Graph::getOrAddNode(int row, int col, char type) {
    for (GraphNode* node : nodes) {
        if (node->row == row && node->col == col) {
            if (node->type == '*') {
                node->type = type;
            }
            return node;
        }
    }

    GraphNode* node = addNode(row, col);
    node->type = type;
    return node;
}

void Graph::addNeighbor(GraphNode* node,
                        GraphNode* neighbor,
                        int costEdge,
                        int costPrediction,
                        char direction,
                        const std::vector<PassedTile>& passedImportant) {
    if (node == nullptr) {
        throw std::out_of_range("Node asal tidak ditemukan.");
    }
    if (neighbor == nullptr) {
        throw std::out_of_range("Node tujuan tidak ditemukan.");
    }

    Edge edge;
    edge.neighbor = neighbor;
    edge.neighborId = neighbor->nodeId;
    edge.costEdge = costEdge;
    edge.costPrediction = costPrediction;
    edge.direction = direction;
    edge.canStopAtNeighbor = true;
    edge.passedImportant = passedImportant;
    node->neighbors.push_back(edge);
}

void Graph::setPredictionCosts(const Board& board) {
    std::vector<PassedTile> importantTiles = board.getImportantTiles();
    importantTargets = importantTiles;
    goal = board.getGoal();
    importantNodeCount = board.getImportantCount();

    for (GraphNode* node : nodes) {
        node->predictionCosts.clear();
        node->finishPredictionCost = board.getDistanceToGoal(node->row, node->col);

        for (const PassedTile& target : importantTiles) {
            PredictionCost prediction;
            prediction.target = target.type;
            prediction.row = target.row;
            prediction.col = target.col;
            prediction.cost = std::abs(node->row - target.row) + std::abs(node->col - target.col);
            node->predictionCosts.push_back(prediction);
        }
    }
}

void Graph::buildFromBoard(const Board& board) {
    clear();

    Position start = board.getStart();
    if (!board.isInside(start.row, start.col)) {
        throw std::runtime_error("Titik awal tidak valid.");
    }

    std::map<std::pair<int, int>, GraphNode*> nodeByPosition;
    std::queue<GraphNode*> frontier;
    std::set<int> processedNodes;

    GraphNode* startNode = getOrAddNode(start.row,
                                        start.col,
                                        board.getType(start.row, start.col));
    root = startNode;
    nodeByPosition[{start.row, start.col}] = startNode;
    frontier.push(startNode);

    while (!frontier.empty()) {
        GraphNode* currentNode = frontier.front();
        frontier.pop();

        if (currentNode == nullptr) {
            throw std::runtime_error("Node graph tidak ditemukan.");
        }
        if (processedNodes.count(currentNode->nodeId) > 0) {
            continue;
        }
        processedNodes.insert(currentNode->nodeId);

        for (int i = 0; i < 4; ++i) {
            SlideResult result = board.slide(currentNode->row,
                                             currentNode->col,
                                             DELTA_ROW[i],
                                             DELTA_COL[i],
                                             DIRECTIONS[i]);
            if (!result.valid) {
                continue;
            }

            std::pair<int, int> stopPosition = {result.stopRow, result.stopCol};
            auto found = nodeByPosition.find(stopPosition);
            GraphNode* nextNode = nullptr;

            if (found == nodeByPosition.end()) {
                nextNode = getOrAddNode(
                    result.stopRow,
                    result.stopCol,
                    board.getType(result.stopRow, result.stopCol));
                nodeByPosition[stopPosition] = nextNode;
                frontier.push(nextNode);
            } else {
                nextNode = found->second;
            }

            addNeighbor(currentNode,
                        nextNode,
                        result.cost,
                        board.getHeuristicCost(result.stopRow, result.stopCol),
                        result.direction,
                        result.passedImportant);
        }
    }

    setPredictionCosts(board);
}

int Graph::getNodeCount() const {
    return static_cast<int>(nodes.size());
}

int Graph::getImportantNodeCount() const {
    return importantNodeCount;
}

int Graph::getPredictionCost(const GraphNode* node, char target) const {
    if (node == nullptr) {
        return 0;
    }

    for (const PredictionCost& prediction : node->predictionCosts) {
        if (prediction.target == target) {
            return prediction.cost;
        }
    }

    return 0;
}

int Graph::getHeuristicCost(const GraphNode* node,
                            int nextNumber,
                            HeuristicMode mode) const {
    if (node == nullptr) {
        return 0;
    }

    if (mode == HeuristicMode::FinishOnly) {
        return node->finishPredictionCost;
    }

    if (mode == HeuristicMode::OrderedSequence) {
        if (nextNumber >= importantNodeCount) {
            return node->finishPredictionCost;
        }

        int total = 0;
        int currentRow = node->row;
        int currentCol = node->col;
        for (int number = nextNumber; number < importantNodeCount; ++number) {
            const PassedTile& target = importantTargets[number];
            total += std::abs(currentRow - target.row) + std::abs(currentCol - target.col);
            currentRow = target.row;
            currentCol = target.col;
        }
        total += std::abs(currentRow - goal.row) + std::abs(currentCol - goal.col);
        return total;
    }

    char target = targetForNextNumber(nextNumber, importantNodeCount);
    if (target == 'O') {
        return node->finishPredictionCost;
    }

    return getPredictionCost(node, target);
}

const GraphNode* Graph::getRoot() const {
    return root;
}

GraphNode* Graph::getRoot() {
    return root;
}

std::vector<GraphNode*> Graph::getNodes() const {
    std::vector<GraphNode*> result;
    result.reserve(nodes.size());
    for (GraphNode* node : nodes) {
        result.push_back(node);
    }
    return result;
}

const GraphNode* Graph::getNode(int nodeId) const {
    if (nodeId < 0 || nodeId >= static_cast<int>(nodes.size())) {
        return nullptr;
    }
    return nodes[nodeId];
}

GraphNode* Graph::getNode(int nodeId) {
    if (nodeId < 0 || nodeId >= static_cast<int>(nodes.size())) {
        return nullptr;
    }
    return nodes[nodeId];
}
