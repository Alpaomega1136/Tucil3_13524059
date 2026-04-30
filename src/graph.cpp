#include "graph.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
const int DELTA_ROW[] = {-1, 0, 1, 0};
const int DELTA_COL[] = {0, 1, 0, -1};
const char DIRECTIONS[] = {'U', 'R', 'D', 'L'};

bool isValidTileType(char type) {
    return type == '*' || type == 'X' || type == 'L' ||
           type == 'Z' || type == 'O' ||
           (type >= '0' && type <= '9');
}

char targetForNextNumber(const std::vector<PredictionCost>& predictions,
                         int nextNumber) {
    if (nextNumber >= 0 && nextNumber <= 9) {
        char target = static_cast<char>('0' + nextNumber);
        for (const PredictionCost& prediction : predictions) {
            if (prediction.target == target) {
                return target;
            }
        }
    }

    return 'O';
}
}

Graph::Graph(int x, int y)
    : rowCount(x), colCount(y), nodes(x, std::vector<typeNode>(y)) {
    if (x <= 0 || y <= 0) {
        throw std::invalid_argument("Ukuran graph harus lebih dari 0.");
    }

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            nodes[row][col].x = col;
            nodes[row][col].y = -row;
        }
    }
}

void Graph::setCost(int x, int y, int value) {
    if (!isInside(x, y)) {
        throw std::out_of_range("Koordinat cost berada di luar peta.");
    }
    nodes[x][y].cost = value;
}

void Graph::setType(int x, int y, char value) {
    if (!isInside(x, y)) {
        throw std::out_of_range("Koordinat tile berada di luar peta.");
    }

    nodes[x][y].type = value;
    if (value == 'Z') {
        start = Position{x, y};
    } else if (value == 'O') {
        goal = Position{x, y};
    }
}

int Graph::getCost(int x, int y) const {
    if (!isInside(x, y)) {
        throw std::out_of_range("Koordinat cost berada di luar peta.");
    }
    return nodes[x][y].cost;
}

char Graph::getType(int x, int y) const {
    if (!isInside(x, y)) {
        throw std::out_of_range("Koordinat tile berada di luar peta.");
    }
    return nodes[x][y].type;
}

int Graph::getRowCount() const {
    return rowCount;
}

int Graph::getColCount() const {
    return colCount;
}

Position Graph::getStart() const {
    return start;
}

Position Graph::getGoal() const {
    return goal;
}

bool Graph::isInside(int x, int y) const {
    return x >= 0 && x < rowCount && y >= 0 && y < colCount;
}

bool Graph::isWall(int x, int y) const {
    return isInside(x, y) && nodes[x][y].type == 'X';
}

bool Graph::isLava(int x, int y) const {
    return isInside(x, y) && nodes[x][y].type == 'L';
}

bool Graph::isWalkable(int x, int y) const {
    return isInside(x, y) && !isWall(x, y) && !isLava(x, y);
}

bool Graph::isImportantTile(int x, int y) const {
    if (!isInside(x, y)) {
        return false;
    }

    char type = nodes[x][y].type;
    return type == 'O' || (type >= '0' && type <= '9');
}

int Graph::getHeuristicCost(int x, int y) const {
    if (!isInside(goal.row, goal.col)) {
        return 0;
    }
    return std::abs(goal.row - x) + std::abs(goal.col - y);
}

std::vector<PassedTile> Graph::getImportantTiles() const {
    std::vector<PassedTile> importantTiles;

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            if (isImportantTile(row, col)) {
                PassedTile tile;
                tile.row = row;
                tile.col = col;
                tile.type = nodes[row][col].type;
                importantTiles.push_back(tile);
            }
        }
    }

    std::sort(importantTiles.begin(),
              importantTiles.end(),
              [](const PassedTile& left, const PassedTile& right) {
                  int leftRank = left.type == 'O' ? 10 : left.type - '0';
                  int rightRank = right.type == 'O' ? 10 : right.type - '0';
                  return leftRank < rightRank;
              });

    return importantTiles;
}

SimpleGraph::~SimpleGraph() {
    clear();
}

void SimpleGraph::clear() {
    for (SimpleNode* node : nodes) {
        delete node;
    }
    nodes.clear();
    root = nullptr;
}

SimpleNode* SimpleGraph::addNode(int x, int y) {
    SimpleNode* node = new SimpleNode;
    node->nodeId = static_cast<int>(nodes.size());
    node->row = x;
    node->col = y;
    node->x = y;
    node->y = -x;
    node->type = '*';
    node->canBranch = false;
    node->finishPredictionCost = 0;

    nodes.push_back(node);
    return node;
}

SimpleNode* SimpleGraph::getOrAddNode(int x, int y, char type, bool canBranch) {
    for (SimpleNode* node : nodes) {
        if (node->row == x && node->col == y) {
            if (node->type == '*') {
                node->type = type;
            }
            node->canBranch = node->canBranch || canBranch;
            return node;
        }
    }

    SimpleNode* node = addNode(x, y);
    node->type = type;
    node->canBranch = canBranch;
    return node;
}

void SimpleGraph::addNeighbor(SimpleNode* node, SimpleNode* neighbor, int costEdge, int costPrediction, char direction, bool canStopAtNeighbor) {
    if (node == nullptr) {
        throw std::out_of_range("Node asal tidak ditemukan.");
    }
    if (neighbor == nullptr) {
        throw std::out_of_range("Node tujuan tidak ditemukan.");
    }

    informationEdge edge;
    edge.neighbor = neighbor;
    edge.neighborId = neighbor->nodeId;
    edge.costEdge = costEdge;
    edge.costPrediction = costPrediction;
    edge.direction = direction;
    edge.canStopAtNeighbor = canStopAtNeighbor;
    node->neighbors.push_back(edge);
}

void SimpleGraph::setPredictionCosts(const Graph& graph) {
    std::vector<PassedTile> importantTiles = graph.getImportantTiles();

    for (SimpleNode* node : nodes) {
        node->predictionCosts.clear();

        for (const PassedTile& target : importantTiles) {
            PredictionCost prediction;
            prediction.target = target.type;
            prediction.row = target.row;
            prediction.col = target.col;
            prediction.cost = std::abs(node->row - target.row) +
                              std::abs(node->col - target.col);
            node->predictionCosts.push_back(prediction);

            if (target.type == 'O') {
                node->finishPredictionCost = prediction.cost;
            }
        }
    }
}

int SimpleGraph::getNodeCount() const {
    return static_cast<int>(nodes.size());
}

int SimpleGraph::getPredictionCost(const SimpleNode* node, char target) const {
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

int SimpleGraph::getHeuristicCost(const SimpleNode* node, int nextNumber, HeuristicMode mode) const {
    if (node == nullptr) {
        return 0;
    }

    if (mode == HeuristicMode::FinishOnly) {
        return node->finishPredictionCost;
    }

    char target = targetForNextNumber(node->predictionCosts, nextNumber);
    return getPredictionCost(node, target);
}

const SimpleNode* SimpleGraph::getRoot() const {
    return root;
}

SimpleNode* SimpleGraph::getRoot() {
    return root;
}

std::vector<SimpleNode*> SimpleGraph::getNodes() const {
    std::vector<SimpleNode*> result;
    result.reserve(nodes.size());
    for (SimpleNode* node : nodes) {
        result.push_back(node);
    }
    return result;
}

const SimpleNode* SimpleGraph::getNode(int nodeId) const {
    if (nodeId < 0 || nodeId >= static_cast<int>(nodes.size())) {
        return nullptr;
    }
    return nodes[nodeId];
}

SimpleNode* SimpleGraph::getNode(int nodeId) {
    if (nodeId < 0 || nodeId >= static_cast<int>(nodes.size())) {
        return nullptr;
    }
    return nodes[nodeId];
}

Graph readMapFromFile(const std::string& filePath) {
    std::ifstream input(filePath);
    if (!input) {
        throw std::runtime_error("File input tidak dapat dibuka.");
    }

    int rowCount = 0;
    int colCount = 0;
    input >> rowCount >> colCount;
    if (!input || rowCount <= 0 || colCount <= 0) {
        throw std::runtime_error("Ukuran peta tidak valid.");
    }

    Graph graph(rowCount, colCount);
    int startCount = 0;
    int goalCount = 0;

    for (int row = 0; row < rowCount; ++row) {
        std::string line;
        input >> line;
        if (static_cast<int>(line.size()) != colCount) {
            throw std::runtime_error("Panjang baris peta tidak sesuai ukuran.");
        }

        for (int col = 0; col < colCount; ++col) {
            char type = line[col];
            if (!isValidTileType(type)) {
                throw std::runtime_error("Karakter peta tidak valid.");
            }

            if (type == 'Z') {
                ++startCount;
            } else if (type == 'O') {
                ++goalCount;
            }
            graph.setType(row, col, type);
        }
    }

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            int cost = 0;
            input >> cost;
            if (!input || cost < 0) {
                throw std::runtime_error("Cost peta tidak valid.");
            }
            graph.setCost(row, col, cost);
        }
    }

    if (startCount != 1) {
        throw std::runtime_error("Peta harus memiliki tepat satu titik awal Z.");
    }
    if (goalCount != 1) {
        throw std::runtime_error("Peta harus memiliki tepat satu titik tujuan O.");
    }

    return graph;
}

SlideResult slide(const Graph& graph, int startX, int startY, int deltaX, int deltaY, char direction) {
    SlideResult result;
    result.direction = direction;

    int currentX = startX;
    int currentY = startY;
    bool hasMoved = false;

    while (true) {
        int nextX = currentX + deltaX;
        int nextY = currentY + deltaY;

        if (!graph.isInside(nextX, nextY)) {
            return SlideResult{};
        }

        if (graph.isWall(nextX, nextY)) {
            if (!hasMoved) {
                return SlideResult{};
            }

            result.valid = true;
            result.stopRow = currentX;
            result.stopCol = currentY;
            return result;
        }

        if (graph.isLava(nextX, nextY)) {
            return SlideResult{};
        }

        currentX = nextX;
        currentY = nextY;
        hasMoved = true;
        result.cost += graph.getCost(currentX, currentY);

        if (graph.isImportantTile(currentX, currentY)) {
            PassedTile tile;
            tile.row = currentX;
            tile.col = currentY;
            tile.type = graph.getType(currentX, currentY);
            result.passedImportant.push_back(tile);
        }
    }
}

void SimpleGraph::convertToSimpleGraph(const Graph& graph) {
    clear();

    Position start = graph.getStart();
    if (!graph.isInside(start.row, start.col)) {
        throw std::runtime_error("Titik awal tidak valid.");
    }

    std::map<std::pair<int, int>, SimpleNode*> nodeByPosition;
    std::queue<SimpleNode*> frontier;
    std::set<int> processedBranchNodes;

    SimpleNode* startNode = getOrAddNode(start.row, start.col, graph.getType(start.row, start.col), true);
    root = startNode;
    nodeByPosition[{start.row, start.col}] = startNode;
    frontier.push(startNode);

    while (!frontier.empty()) {
        SimpleNode* currentNode = frontier.front();
        frontier.pop();

        if (currentNode == nullptr) {
            throw std::runtime_error("Node simple graph tidak ditemukan.");
        }
        if (processedBranchNodes.count(currentNode->nodeId) > 0) {
            continue;
        }
        processedBranchNodes.insert(currentNode->nodeId);

        int currentX = currentNode->row;
        int currentY = currentNode->col;

        for (int i = 0; i < 4; ++i) {
            SlideResult result = slide(graph, currentX, currentY, DELTA_ROW[i], DELTA_COL[i], DIRECTIONS[i]);
            if (!result.valid) {
                continue;
            }

            std::pair<int, int> stopPosition = {result.stopRow, result.stopCol};
            auto found = nodeByPosition.find(stopPosition);
            SimpleNode* nextNode = nullptr;

            if (found == nodeByPosition.end()) {
                nextNode = getOrAddNode(result.stopRow, result.stopCol, graph.getType(result.stopRow, result.stopCol), true);
                nodeByPosition[stopPosition] = nextNode;
                frontier.push(nextNode);
            } else {
                nextNode = found->second;
            }

            addNeighbor(currentNode, nextNode, result.cost, graph.getHeuristicCost(result.stopRow, result.stopCol),
                        result.direction, true);
            currentNode->neighbors.back().passedImportant = result.passedImportant;
        }
    }

    setPredictionCosts(graph);
}
