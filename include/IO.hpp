#pragma once

#include "Board.hpp"

#include <iosfwd>
#include <string>
#include <vector>

bool fileExists(const std::string& path);
std::string getProjectPath(const std::string& path);
std::string resolveInputPath(const std::string& path);
std::string getOutputPath(const std::string& inputPath);
std::string getOutputPath(const std::string& inputPath, const std::string& algorithm, const std::string& heuristic);
std::string directionsToString(const std::vector<char>& path);

void writeBoardWithActor(std::ostream& output, const Board& board, Position actor);
void writeSolutionSteps(std::ostream& output, const Board& board, const std::vector<char>& path, const std::vector<Position>& positions);
void saveSolution(const Board& board, const std::vector<char>& path, const std::vector<Position>& positions, int totalCost,
                int totalIterations, long long elapsedMs, const std::string& outputPath);
void saveSolution(const Board& board, const std::vector<char>& path, const std::vector<Position>& positions, int totalCost,
                int totalIterations, long long elapsedMs, const std::string& outputPath, const std::string& algorithm,
                const std::string& heuristic);
