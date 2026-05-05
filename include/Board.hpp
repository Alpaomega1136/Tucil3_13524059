#pragma once

#include <string>
#include <vector>

struct Tile {
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

struct SlideResult {
    bool valid = false;
    int stopRow = 0;
    int stopCol = 0;
    int cost = 0;
    char direction = '?';
    std::vector<PassedTile> passedImportant;
};

class Board {
private:
    int rowCount = 0;
    int colCount = 0;
    std::vector<std::vector<Tile>> tiles;
    Position start{-1, -1};
    Position goal{-1, -1};
    int importantCount = 0;

public:
    Board() = default;
    Board(int rows, int cols);

    void setCost(int row, int col, int value);
    void setType(int row, int col, char value);
    void setImportantCount(int value);

    int getCost(int row, int col) const;
    char getType(int row, int col) const;
    int getRowCount() const;
    int getColCount() const;
    int getImportantCount() const;
    Position getStart() const;
    Position getGoal() const;

    bool isInside(int row, int col) const;
    bool isWall(int row, int col) const;
    bool isLava(int row, int col) const;
    bool isWalkable(int row, int col) const;
    bool isImportantTile(int row, int col) const;
    bool isGoal(int row, int col) const;

    int getHeuristicCost(int row, int col) const;
    int getDistanceToGoal(int row, int col) const;
    std::vector<PassedTile> getImportantTiles() const;
    SlideResult slide(int startRow, int startCol, int deltaRow, int deltaCol, char direction) const;
};

Board readBoardFromFile(const std::string& filePath);
