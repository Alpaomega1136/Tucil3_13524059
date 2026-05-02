#pragma once

#include "graph.hpp"

#include <vector>

class PathFinding {
private:
    std::vector<char> path;
    std::vector<Position> pathPositions;
    int totalCost = 0;
    int totalIterations = 0;

    enum class Algorithm { UCS, AStar, GBFS };

    void search(const Graph& graph, Algorithm algorithm, HeuristicMode mode);

public:
    PathFinding() = default;
    ~PathFinding() = default;

    std::vector<char> getPath() const;
    std::vector<Position> getPathPositions() const;
    int getTotalCost() const;
    int getTotalIterations() const;

    void UCS(const Graph& graph);
    void AStar(const Graph& graph, HeuristicMode mode = HeuristicMode::FinishOnly);
    void GBFS(const Graph& graph, HeuristicMode mode = HeuristicMode::FinishOnly);
};
