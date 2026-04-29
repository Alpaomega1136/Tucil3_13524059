#include "graph.hpp"

#include <cmath>
#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
bool isValidTileType(char type) {
    return type == '*' || type == 'X' || type == 'L' ||
           type == 'Z' || type == 'O' ||
           (type >= '0' && type <= '9');
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
    int costFromPreviousImportant = 0;

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
            if (!result.points.empty() &&
                result.points.back().row == currentX &&
                result.points.back().col == currentY) {
                result.points.back().isStopPoint = true;
            } else {
                SlidePoint point;
                point.row = currentX;
                point.col = currentY;
                point.costFromPrevious = costFromPreviousImportant;
                point.type = graph.getType(currentX, currentY);
                point.isStopPoint = true;
                result.points.push_back(point);
            }
            return result;
        }

        if (graph.isLava(nextX, nextY)) {
            return SlideResult{};
        }

        currentX = nextX;
        currentY = nextY;
        hasMoved = true;
        costFromPreviousImportant += graph.getCost(currentX, currentY);

        if (graph.isImportantTile(currentX, currentY)) {
            SlidePoint point;
            point.row = currentX;
            point.col = currentY;
            point.costFromPrevious = costFromPreviousImportant;
            point.type = graph.getType(currentX, currentY);
            point.isStopPoint = false;
            result.points.push_back(point);
            costFromPreviousImportant = 0;
        }
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
    std::set<int> processedBranchNodes;

    SimpleNode* startNode = getOrAddNode(start.x, start.y, graph.getType(start.x, start.y), true);
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
        if (processedBranchNodes.count(currentNode->nodeId) > 0) {
            continue;
        }
        processedBranchNodes.insert(currentNode->nodeId);

        int currentX = currentNode->row;
        int currentY = currentNode->col;

        for (int i = 0; i < 4; ++i) {
            SlideResult result = slide(graph, currentX, currentY,deltaX[i], deltaY[i], directions[i]);
            if (!result.valid) {
                continue;
            }

            SimpleNode* previousNode = currentNode;
            for (const SlidePoint& point : result.points) {
                std::pair<int, int> position = {point.row, point.col};
                auto found = nodeByPosition.find(position);
                SimpleNode* nextNode = nullptr;

                if (found == nodeByPosition.end()) {
                    nextNode = getOrAddNode(point.row, point.col, point.type, point.isStopPoint);
                    nodeByPosition[position] = nextNode;
                    if (point.isStopPoint) {
                        frontier.push(nextNode);
                    }
                } else {
                    nextNode = found->second;
                    bool wasBranchNode = nextNode->canBranch;
                    if (nextNode->type == '*') {
                        nextNode->type = point.type;
                    }
                    nextNode->canBranch = nextNode->canBranch || point.isStopPoint;
                    if (!wasBranchNode && nextNode->canBranch) {
                        frontier.push(nextNode);
                    }
                }

                addNeighbor(previousNode, nextNode, point.costFromPrevious, graph.getHeuristicCost(point.row, point.col), result.direction, point.isStopPoint);
                previousNode = nextNode;
            }
        }
    }
}
