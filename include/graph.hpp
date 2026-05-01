#pragma once

#include "Board.hpp"

#include <vector>

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

struct GraphNode;

struct Edge {
    GraphNode* neighbor = nullptr;
    int neighborId = -1;
    int costEdge = 0;
    int costPrediction = 0;
    char direction = '?';
    bool canStopAtNeighbor = true;
    std::vector<PassedTile> passedImportant;
};

struct GraphNode {
    int nodeId = -1;
    int row = 0;
    int col = 0;
    int x = 0;
    int y = 0;
    char type = '*';
    bool canBranch = true;
    int finishPredictionCost = 0;
    std::vector<PredictionCost> predictionCosts;
    std::vector<Edge> neighbors;
};

class Graph {
private:
    std::vector<GraphNode*> nodes;
    GraphNode* root = nullptr;

public:
    Graph() = default;
    ~Graph();
    Graph(const Graph& other) = delete;
    Graph& operator=(const Graph& other) = delete;

    void clear();
    GraphNode* addNode(int row, int col);
    GraphNode* getOrAddNode(int row, int col, char type);
    void addNeighbor(GraphNode* node, GraphNode* neighbor, int costEdge, int costPrediction, char direction, const std::vector<PassedTile>& passedImportant);
    void setPredictionCosts(const Board& board);
    void buildFromBoard(const Board& board);

    int getNodeCount() const;
    int getPredictionCost(const GraphNode* node, char target) const;
    int getHeuristicCost(const GraphNode* node, int nextNumber, HeuristicMode mode) const;
    const GraphNode* getRoot() const;
    GraphNode* getRoot();
    std::vector<GraphNode*> getNodes() const;
    const GraphNode* getNode(int nodeId) const;
    GraphNode* getNode(int nodeId);
};
