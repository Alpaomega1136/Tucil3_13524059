#include "Board.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <future>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {
bool isValidTileType(char type) {
    return type == '*' || type == 'X' || type == 'L' ||
           type == 'Z' || type == 'O' ||
           (type >= '0' && type <= '9');
}

struct ParsedMapRow {
    std::vector<char> types;
    int startCount = 0;
    int goalCount = 0;
    std::vector<bool> foundNumbers = std::vector<bool>(10, false);
    int maxNumber = -1;
};

struct ParsedCostRow {
    std::vector<int> costs;
};

ParsedMapRow parseMapRow(const std::string& line, int colCount) {
    if (line.empty()) {
        throw std::runtime_error("Baris peta tidak boleh kosong.");
    }
    if (static_cast<int>(line.size()) != colCount) {
        throw std::runtime_error("Panjang baris peta tidak sesuai ukuran.");
    }

    ParsedMapRow parsed;
    parsed.types.resize(colCount);

    for (int col = 0; col < colCount; ++col) {
        char type = line[col];
        if (!isValidTileType(type)) {
            throw std::runtime_error("Karakter peta tidak valid.");
        }

        if (type == 'Z') {
            ++parsed.startCount;
        } else if (type == 'O') {
            ++parsed.goalCount;
        } else if (type >= '0' && type <= '9') {
            int number = type - '0';
            parsed.foundNumbers[number] = true;
            parsed.maxNumber = std::max(parsed.maxNumber, number);
        }

        parsed.types[col] = type;
    }

    return parsed;
}

ParsedCostRow parseCostRow(const std::string& line, int colCount) {
    if (line.empty()) {
        throw std::runtime_error("Baris cost tidak boleh kosong.");
    }

    ParsedCostRow parsed;
    parsed.costs.resize(colCount);
    std::istringstream costLine(line);

    for (int col = 0; col < colCount; ++col) {
        int cost = 0;
        if (!(costLine >> cost) || cost < 0) {
            throw std::runtime_error("Cost peta tidak valid.");
        }
        parsed.costs[col] = cost;
    }

    std::string extraCost;
    if (costLine >> extraCost) {
        throw std::runtime_error("Jumlah cost pada baris tidak sesuai ukuran.");
    }

    return parsed;
}

int getWorkerCount(int itemCount) {
    unsigned int hardwareThreads = std::thread::hardware_concurrency();
    if (hardwareThreads == 0) {
        hardwareThreads = 2;
    }
    return std::max(1, std::min(itemCount, static_cast<int>(hardwareThreads)));
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

void Board::setImportantCount(int value) {
    importantCount = value;
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

int Board::getImportantCount() const {
    return importantCount;
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
    return type >= '0' && type <= '9';
}

bool Board::isGoal(int row, int col) const {
    return isInside(row, col) && tiles[row][col].type == 'O';
}

int Board::getHeuristicCost(int row, int col) const {
    return getDistanceToGoal(row, col);
}

int Board::getDistanceToGoal(int row, int col) const {
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
                importantTiles.push_back(
                    PassedTile{row, col, tiles[row][col].type});
            }
        }
    }

    std::sort(importantTiles.begin(), importantTiles.end(),
              [](const PassedTile& left, const PassedTile& right) {
                  return left.type < right.type;
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

        if (isImportantTile(currentRow, currentCol) ||
            isGoal(currentRow, currentCol)) {
            result.passedImportant.push_back(
                PassedTile{currentRow,
                           currentCol,
                           getType(currentRow, currentCol)});
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
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Board board(rowCount, colCount);
    int startCount = 0;
    int goalCount = 0;
    std::vector<bool> foundNumbers(10, false);
    int maxNumber = -1;
    std::vector<std::string> mapLines(rowCount);
    std::vector<std::string> costLines(rowCount);

    for (int row = 0; row < rowCount; ++row) {
        if (!std::getline(input, mapLines[row])) {
            throw std::runtime_error("Jumlah baris peta kurang dari ukuran.");
        }
    }

    for (int row = 0; row < rowCount; ++row) {
        if (!std::getline(input, costLines[row])) {
            throw std::runtime_error("Jumlah baris cost kurang dari ukuran.");
        }
    }

    std::string extraInput;
    if (input >> extraInput) {
        throw std::runtime_error("Input memiliki data tambahan setelah matriks cost.");
    }

    std::vector<ParsedMapRow> parsedMapRows(rowCount);
    std::vector<std::future<void>> mapFutures;
    int mapWorkerCount = getWorkerCount(rowCount);
    mapFutures.reserve(mapWorkerCount);

    for (int worker = 0; worker < mapWorkerCount; ++worker) {
        int begin = worker * rowCount / mapWorkerCount;
        int end = (worker + 1) * rowCount / mapWorkerCount;
        mapFutures.push_back(std::async(std::launch::async, [&mapLines, &parsedMapRows, colCount, begin, end]() {
            for (int row = begin; row < end; ++row) {
                parsedMapRows[row] = parseMapRow(mapLines[row], colCount);
            }
        }));
    }

    for (std::future<void>& future : mapFutures) {
        future.get();
    }

    for (int row = 0; row < rowCount; ++row) {
        const ParsedMapRow& parsed = parsedMapRows[row];
        startCount += parsed.startCount;
        goalCount += parsed.goalCount;
        maxNumber = std::max(maxNumber, parsed.maxNumber);
        for (int number = 0; number < 10; ++number) {
            foundNumbers[number] = foundNumbers[number] || parsed.foundNumbers[number];
        }

        for (int col = 0; col < colCount; ++col) {
            board.setType(row, col, parsed.types[col]);
        }
    }

    if (startCount != 1) {
        throw std::runtime_error("Peta harus memiliki tepat satu titik awal Z.");
    }
    if (goalCount != 1) {
        throw std::runtime_error("Peta harus memiliki tepat satu titik tujuan O.");
    }

    for (int number = 0; number <= maxNumber; ++number) {
        if (!foundNumbers[number]) {
            throw std::runtime_error("Angka pada peta harus berurutan mulai dari 0.");
        }
    }
    board.setImportantCount(maxNumber + 1);

    std::vector<ParsedCostRow> parsedCostRows(rowCount);
    std::vector<std::future<void>> costFutures;
    int costWorkerCount = getWorkerCount(rowCount);
    costFutures.reserve(costWorkerCount);

    for (int worker = 0; worker < costWorkerCount; ++worker) {
        int begin = worker * rowCount / costWorkerCount;
        int end = (worker + 1) * rowCount / costWorkerCount;
        costFutures.push_back(std::async(std::launch::async, [&costLines, &parsedCostRows, colCount, begin, end]() {
            for (int row = begin; row < end; ++row) {
                parsedCostRows[row] = parseCostRow(costLines[row], colCount);
            }
        }));
    }

    for (std::future<void>& future : costFutures) {
        future.get();
    }

    for (int row = 0; row < rowCount; ++row) {
        const ParsedCostRow& parsed = parsedCostRows[row];
        for (int col = 0; col < colCount; ++col) {
            board.setCost(row, col, parsed.costs[col]);
        }
    }

    return board;
}
