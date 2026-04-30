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
    int row = 0;
    int col = 0;

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

struct PassedTile {
    int row = 0;
    int col = 0;
    char type = '*';
};

struct PredictionCost {
    char target = '*';
    int row = 0;
    int col = 0;
    int cost = 0;
};

enum class HeuristicMode {
    ImportantSequence,
    FinishOnly
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
    bool isImportantTile(int x, int y) const;
    int getHeuristicCost(int x, int y) const;
    std::vector<PassedTile> getImportantTiles() const;
};

struct SimpleNode;

struct informationEdge {
    SimpleNode* neighbor = nullptr;
    int neighborId = -1;
    int costEdge = 0;
    int costPrediction = 0;
    char direction = '?';
    bool canStopAtNeighbor = false;
    std::vector<PassedTile> passedImportant;
};

struct SimpleNode {
    int nodeId = -1;
    int row = 0;
    int col = 0;
    int x = 0;
    int y = 0;
    char type = '*';
    bool canBranch = false;
    int finishPredictionCost = 0;
    std::vector<PredictionCost> predictionCosts;
    std::vector<informationEdge> neighbors;
};

struct SlideResult {
    bool valid = false;
    int stopRow = 0;
    int stopCol = 0;
    int cost = 0;
    char direction = '?';
    std::vector<PassedTile> passedImportant;
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
    SimpleNode* addNode(int row, int col);
    SimpleNode* getOrAddNode(int row, int col, char type, bool canBranch);
    void addNeighbor(SimpleNode* node, SimpleNode* neighbor, int costEdge, int costPrediction, char direction, bool canStopAtNeighbor);
    void setPredictionCosts(const Graph& graph);
    void convertToSimpleGraph(const Graph& graph);

    int getNodeCount() const;
    int getPredictionCost(const SimpleNode* node, char target) const;
    int getHeuristicCost(const SimpleNode* node,
                         int nextNumber,
                         HeuristicMode mode) const;
    const SimpleNode* getRoot() const;
    SimpleNode* getRoot();
    std::vector<SimpleNode*> getNodes() const;
    const SimpleNode* getNode(int nodeId) const;
    SimpleNode* getNode(int nodeId);
};

Graph readMapFromFile(const std::string& filePath);
SlideResult slide(const Graph& graph, int startRow,int startCol,int deltaRow,int deltaCol, char direction);
