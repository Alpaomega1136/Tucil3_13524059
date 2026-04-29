#include "graph.hpp"

#include <cmath>
#include <fstream>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>

Graph::Graph(int x, int y) : rowCount(x), colCount(y),nodes(x, std::vector<typeNode>(y)) {
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

int Graph::getHeuristicCost(int x, int y) const {
    if (!isInside(goal.x, goal.y)) {
        return 0;
    }
    return std::abs(goal.x - x) + std::abs(goal.y - y);
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

    nodes.push_back(node);
    return node;
}

void SimpleGraph::addNeighbor(SimpleNode* node, SimpleNode* neighbor, int costEdge, int costPrediction, char direction) {
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
    node->neighbors.push_back(edge);
}

int SimpleGraph::getNodeCount() const {
    return static_cast<int>(nodes.size());
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
            bool validType =type == '*' || type == 'X' || type == 'L' ||
                            type == 'Z' || type == 'O' ||
                            (type >= '0' && type <= '9');
            if (!validType) {
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
            result.stopX = currentX;
            result.stopY = currentY;
            return result;
        }

        if (graph.isLava(nextX, nextY)) {
            return SlideResult{};
        }

        currentX = nextX;
        currentY = nextY;
        hasMoved = true;
        result.cost += graph.getCost(currentX, currentY);
    }
}

void SimpleGraph::convertToSimpleGraph(const Graph& graph) {
    clear();

    Position start = graph.getStart();
    if (!graph.isInside(start.x, start.y)) {
        throw std::runtime_error("Titik awal tidak valid.");
    }

    std::map<std::pair<int, int>, SimpleNode*> nodeByPosition;
    std::queue<SimpleNode*> frontier;

    SimpleNode* startNode = addNode(start.x, start.y);
    root = startNode;
    nodeByPosition[{start.x, start.y}] = startNode;
    frontier.push(startNode);

    const int deltaX[] = {-1, 0, 1, 0};
    const int deltaY[] = {0, 1, 0, -1};
    const char directions[] = {'U', 'R', 'D', 'L'};

    while (!frontier.empty()) {
        SimpleNode* currentNode = frontier.front();
        frontier.pop();

        if (currentNode == nullptr) {
            throw std::runtime_error("Node simple graph tidak ditemukan.");
        }
        int currentX = currentNode->row;
        int currentY = currentNode->col;

        for (int i = 0; i < 4; ++i) {
            SlideResult result = slide(graph, currentX, currentY,deltaX[i], deltaY[i], directions[i]);
            if (!result.valid) { // when get gameover because slide not out of range or hit lava
                continue;
            }

            std::pair<int, int> stopPosition = {result.stopX, result.stopY};
            auto found = nodeByPosition.find(stopPosition);
            SimpleNode* neighbor = nullptr;

            if (found == nodeByPosition.end()) {
                neighbor = addNode(result.stopX, result.stopY);
                nodeByPosition[stopPosition] = neighbor;
                frontier.push(neighbor);
            } else {
                neighbor = found->second;
            }

            addNeighbor(currentNode, neighbor, result.cost, graph.getHeuristicCost(result.stopX, result.stopY), result.direction);
        }
    }
}
