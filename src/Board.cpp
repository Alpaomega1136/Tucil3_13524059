#include "Board.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {
bool isValidTileType(char type) {
    return type == '*' || type == 'X' || type == 'L' ||
           type == 'Z' || type == 'O' ||
           (type >= '0' && type <= '9');
}
}

Board::Board(int rows, int cols)
    : rowCount(rows), colCount(cols), tiles(rows, std::vector<Tile>(cols)) {
    if (rows <= 0 || cols <= 0) {
        throw std::invalid_argument("Ukuran board harus lebih dari 0.");
    }

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            tiles[row][col].x = col;
            tiles[row][col].y = -row;
        }
    }
}

void Board::setCost(int row, int col, int value) {
    if (!isInside(row, col)) {
        throw std::out_of_range("Koordinat cost berada di luar peta.");
    }
    tiles[row][col].cost = value;
}

void Board::setType(int row, int col, char value) {
    if (!isInside(row, col)) {
        throw std::out_of_range("Koordinat tile berada di luar peta.");
    }

    tiles[row][col].type = value;
    if (value == 'Z') {
        start = Position{row, col};
    } else if (value == 'O') {
        goal = Position{row, col};
    }
}

int Board::getCost(int row, int col) const {
    if (!isInside(row, col)) {
        throw std::out_of_range("Koordinat cost berada di luar peta.");
    }
    return tiles[row][col].cost;
}

char Board::getType(int row, int col) const {
    if (!isInside(row, col)) {
        throw std::out_of_range("Koordinat tile berada di luar peta.");
    }
    return tiles[row][col].type;
}

int Board::getRowCount() const {
    return rowCount;
}

int Board::getColCount() const {
    return colCount;
}

Position Board::getStart() const {
    return start;
}

Position Board::getGoal() const {
    return goal;
}

bool Board::isInside(int row, int col) const {
    return row >= 0 && row < rowCount && col >= 0 && col < colCount;
}

bool Board::isWall(int row, int col) const {
    return isInside(row, col) && tiles[row][col].type == 'X';
}

bool Board::isLava(int row, int col) const {
    return isInside(row, col) && tiles[row][col].type == 'L';
}

bool Board::isWalkable(int row, int col) const {
    return isInside(row, col) && !isWall(row, col) && !isLava(row, col);
}

bool Board::isImportantTile(int row, int col) const {
    if (!isInside(row, col)) {
        return false;
    }

    char type = tiles[row][col].type;
    return type == 'O' || (type >= '0' && type <= '9');
}

int Board::getHeuristicCost(int row, int col) const {
    if (!isInside(goal.row, goal.col)) {
        return 0;
    }
    return std::abs(goal.row - row) + std::abs(goal.col - col);
}

std::vector<PassedTile> Board::getImportantTiles() const {
    std::vector<PassedTile> importantTiles;

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            if (isImportantTile(row, col)) {
                importantTiles.push_back(PassedTile{row, col, tiles[row][col].type});
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

SlideResult Board::slide(int startRow,
                         int startCol,
                         int deltaRow,
                         int deltaCol,
                         char direction) const {
    SlideResult result;
    result.direction = direction;

    int currentRow = startRow;
    int currentCol = startCol;
    bool hasMoved = false;

    while (true) {
        int nextRow = currentRow + deltaRow;
        int nextCol = currentCol + deltaCol;

        if (!isInside(nextRow, nextCol)) {
            return SlideResult{};
        }

        if (isWall(nextRow, nextCol)) {
            if (!hasMoved) {
                return SlideResult{};
            }

            result.valid = true;
            result.stopRow = currentRow;
            result.stopCol = currentCol;
            return result;
        }

        if (isLava(nextRow, nextCol)) {
            return SlideResult{};
        }

        currentRow = nextRow;
        currentCol = nextCol;
        hasMoved = true;
        result.cost += getCost(currentRow, currentCol);

        if (isImportantTile(currentRow, currentCol)) {
            result.passedImportant.push_back(
                PassedTile{currentRow, currentCol, getType(currentRow, currentCol)});
        }
    }
}

Board readBoardFromFile(const std::string& filePath) {
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

    Board board(rowCount, colCount);
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
            board.setType(row, col, type);
        }
    }

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            int cost = 0;
            input >> cost;
            if (!input || cost < 0) {
                throw std::runtime_error("Cost peta tidak valid.");
            }
            board.setCost(row, col, cost);
        }
    }

    if (startCount != 1) {
        throw std::runtime_error("Peta harus memiliki tepat satu titik awal Z.");
    }
    if (goalCount != 1) {
        throw std::runtime_error("Peta harus memiliki tepat satu titik tujuan O.");
    }

    return board;
}
