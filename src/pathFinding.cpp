#include "pathFinding.hpp"

#include <algorithm>
#include <map>
#include <queue>
#include <utility>
#include <vector>

namespace {

struct StateKey {
    int nodeId;
    int nextNumber;

    bool operator<(const StateKey& other) const {
        if (nodeId != other.nodeId) {
            return nodeId < other.nodeId;
        }
        return nextNumber < other.nextNumber;
    }
};

struct HistoryRecord {
    int nodeId = -1;
    int parent = -1;
    char direction = '?';
};

struct FrontierEntry {
    int priority = 0;
    int gCost = 0;
    int historyIdx = -1;
    int nodeId = -1;
    int nextNumber = 0;
};

struct FrontierCompare {
    bool operator()(const FrontierEntry& left, const FrontierEntry& right) const {
        return left.priority > right.priority;
    }
};

bool advanceImportantSequence(int currentNext, const std::vector<PassedTile>& passedTiles, int& nextAfter) {
    nextAfter = currentNext;
    for (const PassedTile& tile : passedTiles) {
        if (tile.type < '0' || tile.type > '9') {
            continue;
        }

        int digit = tile.type - '0';
        if (digit < nextAfter) {
            continue;
        }
        if (digit == nextAfter) {
            ++nextAfter;
            continue;
        }
        return false;
    }
    return true;
}

}

std::vector<char> PathFinding::getPath() const {
    return path;
}

std::vector<Position> PathFinding::getPathPositions() const {
    return pathPositions;
}

int PathFinding::getTotalCost() const {
    return totalCost;
}

int PathFinding::getTotalIterations() const {
    return totalIterations;
}

void PathFinding::search(const Graph& graph, Algorithm algorithm, HeuristicMode mode) {
    path.clear();
    pathPositions.clear();
    totalCost = 0;
    totalIterations = 0;

    const GraphNode* root = graph.getRoot();
    if (root == nullptr) {
        return;
    }

    const int importantCount = graph.getImportantNodeCount();
    const bool useImportantSequence = mode == HeuristicMode::ImportantSequence;

    std::vector<HistoryRecord> history;
    std::priority_queue<FrontierEntry, std::vector<FrontierEntry>, FrontierCompare> frontier;
    std::map<StateKey, int> bestCost;

    int rootHeuristic = 0;
    if (algorithm != Algorithm::UCS) {
        rootHeuristic = graph.getHeuristicCost(root, 0, mode);
    }

    history.push_back(HistoryRecord{root->nodeId, -1, '?'});

    FrontierEntry rootEntry;
    rootEntry.gCost = 0;
    rootEntry.historyIdx = 0;
    rootEntry.nodeId = root->nodeId;
    rootEntry.nextNumber = 0;
    if (algorithm == Algorithm::UCS) {
        rootEntry.priority = 0;
    } else if (algorithm == Algorithm::GBFS) {
        rootEntry.priority = rootHeuristic;
    } else {
        rootEntry.priority = rootHeuristic;
    }

    frontier.push(rootEntry);
    bestCost[StateKey{root->nodeId, 0}] = 0;

    int goalHistoryIdx = -1;
    int goalCost = 0;

    while (!frontier.empty()) {
        FrontierEntry current = frontier.top();
        frontier.pop();
        ++totalIterations;

        StateKey currentKey{current.nodeId, current.nextNumber};
        auto bestIt = bestCost.find(currentKey);
        if (bestIt != bestCost.end() && bestIt->second < current.gCost) {
            continue;
        }

        const GraphNode* currentNode = graph.getNode(current.nodeId);
        if (currentNode == nullptr) {
            continue;
        }

        bool reachedFinish = currentNode->type == 'O';
        bool completedImportantSequence = current.nextNumber == importantCount;
        if (reachedFinish && (!useImportantSequence || completedImportantSequence)) {
            goalHistoryIdx = current.historyIdx;
            goalCost = current.gCost;
            break;
        }

        for (const Edge& edge : currentNode->neighbors) {
            int nextNumberAfter = current.nextNumber;
            if (useImportantSequence) {
                if (!advanceImportantSequence(current.nextNumber, edge.passedImportant, nextNumberAfter)) {
                    continue;
                }
            } else {
                nextNumberAfter = 0;
            }

            int newGCost = current.gCost + edge.costEdge;
            StateKey neighborKey{edge.neighborId, nextNumberAfter};
            auto neighborIt = bestCost.find(neighborKey);
            if (neighborIt != bestCost.end() && neighborIt->second <= newGCost) {
                continue;
            }
            bestCost[neighborKey] = newGCost;

            int neighborHeuristic = 0;
            if (algorithm != Algorithm::UCS) {
                neighborHeuristic = graph.getHeuristicCost(edge.neighbor, nextNumberAfter, mode);
            }

            int newPriority = 0;
            if (algorithm == Algorithm::UCS) {
                newPriority = newGCost;
            } else if (algorithm == Algorithm::GBFS) {
                newPriority = neighborHeuristic;
            } else {
                newPriority = newGCost + neighborHeuristic;
            }

            history.push_back(HistoryRecord{edge.neighborId, current.historyIdx, edge.direction});
            FrontierEntry nextEntry;
            nextEntry.priority = newPriority;
            nextEntry.gCost = newGCost;
            nextEntry.historyIdx = static_cast<int>(history.size()) - 1;
            nextEntry.nodeId = edge.neighborId;
            nextEntry.nextNumber = nextNumberAfter;
            frontier.push(nextEntry);
        }
    }

    if (goalHistoryIdx == -1) {
        return;
    }

    totalCost = goalCost;

    std::vector<char> directions;
    std::vector<Position> positions;
    int cursor = goalHistoryIdx;
    while (cursor != -1) {
        const HistoryRecord& record = history[cursor];
        const GraphNode* node = graph.getNode(record.nodeId);
        if (node != nullptr) {
            positions.push_back(Position{node->row, node->col});
        }
        if (record.parent != -1) {
            directions.push_back(record.direction);
        }
        cursor = record.parent;
    }

    std::reverse(directions.begin(), directions.end());
    std::reverse(positions.begin(), positions.end());

    path = directions;
    pathPositions = positions;
}

void PathFinding::UCS(const Graph& graph) {
    search(graph, Algorithm::UCS, HeuristicMode::FinishOnly);
}

void PathFinding::AStar(const Graph& graph, HeuristicMode mode) {
    search(graph, Algorithm::AStar, mode);
}

void PathFinding::GBFS(const Graph& graph, HeuristicMode mode) {
    search(graph, Algorithm::GBFS, mode);
}
