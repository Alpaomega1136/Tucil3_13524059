#pragma once

#include <string>
#include <vector>

struct typeNode {
    int x = 0;
    int y = 0;
    int cost = 0;
    char type = '*';
};

struct Position {
    int x = 0;
    int y = 0;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

class Graph {
private:
    int rowCount = 0;
    int colCount = 0;
    std::vector<std::vector<typeNode>> nodes;
    Position start{-1, -1};
    Position goal{-1, -1};

public:
    Graph() = default;
    Graph(int x, int y);

    void setCost(int x, int y, int value);
    void setType(int x, int y, char value);

    int getCost(int x, int y) const;
    char getType(int x, int y) const;
    int getRowCount() const;
    int getColCount() const;
    Position getStart() const;
    Position getGoal() const;

    bool isInside(int x, int y) const;
    bool isWall(int x, int y) const;
    bool isLava(int x, int y) const;
    bool isWalkable(int x, int y) const;
    int getHeuristicCost(int x, int y) const;
};

struct SimpleNode;

struct informationEdge {
    SimpleNode* neighbor = nullptr;
    int neighborId = -1;
    int costEdge = 0;
    int costPrediction = 0;
    char direction = '?';
};

struct SimpleNode {
    int nodeId = -1;
    int row = 0;
    int col = 0;
    int x = 0;
    int y = 0;
    std::vector<informationEdge> neighbors;
};

struct SlideResult {
    bool valid = false;
    int stopX = 0;
    int stopY = 0;
    int cost = 0;
    char direction = '?';
};

class SimpleGraph {
private:
    std::vector<SimpleNode*> nodes;
    SimpleNode* root = nullptr;

public:
    SimpleGraph() = default;
    ~SimpleGraph();
    SimpleGraph(const SimpleGraph& other) = delete;
    SimpleGraph& operator=(const SimpleGraph& other) = delete;

    void clear();
    SimpleNode* addNode(int x, int y);
    void addNeighbor(SimpleNode* node, SimpleNode* neighbor, int costEdge, int costPrediction, char direction);
    void convertToSimpleGraph(const Graph& graph);

    int getNodeCount() const;
    const SimpleNode* getRoot() const;
    SimpleNode* getRoot();
    std::vector<SimpleNode*> getNodes() const;
    const SimpleNode* getNode(int nodeId) const;
    SimpleNode* getNode(int nodeId);
};

Graph readMapFromFile(const std::string& filePath);
SlideResult slide(const Graph& graph, int startX, int startY, int deltaX, int deltaY, char direction);
