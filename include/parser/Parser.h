#ifndef DA_PROJECT2_PARSER_H
#define DA_PROJECT2_PARSER_H

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "data_structures/Graph.h"

/**
 * @brief A single program line in a live range.
 */
struct ProgramPoint {
    int line = 0;
    bool begins = false;
    bool ends = false;
};

/**
 * @brief One live range read directly from the ranges input file.
 */
struct LiveRange {
    std::string variable;
    std::vector<ProgramPoint> points;
};

/**
 * @brief Register count and selected allocation algorithm read from settings.
 */
struct RegisterSettings {
    int registers = 0;
    std::string algorithm = "basic";
    int parameter = 0;
};

/**
 * @brief A live web created by merging overlapping live ranges of one variable.
 */
struct Web {
    int id = 0;
    std::string variable;
    std::vector<ProgramPoint> points;
    std::set<int> liveLines;
};

/**
 * @brief Parsed input data and derived structures used by later tasks.
 */
struct ParsedInput {
    std::vector<LiveRange> ranges;
    RegisterSettings settings;
    std::vector<Web> webs;
    Graph<int> interferenceGraph;
};

/**
 * @brief Final allocation result shown in the menu and written to output files.
 */
struct AllocationResult {
    bool success = false;
    int registersUsed = 0;
    std::vector<Web> webs;
    std::map<int, std::set<int>> interference;
    std::map<int, int> webToRegister;
    std::set<int> spilledWebs;
    std::string warning;
};

ProgramPoint parseProgramPoint(const std::string &token, int lineNumber);
std::vector<LiveRange> parseRangesFile(const std::string &path);
RegisterSettings parseRegistersFile(const std::string &path);
bool rangesOverlap(const LiveRange &a, const LiveRange &b);
std::vector<Web> buildWebs(const std::vector<LiveRange> &ranges);
void buildInterferenceGraph(const std::vector<Web> &webs, Graph<int> &graph);
std::map<int, std::set<int>> graphToInterferenceMap(const Graph<int> &graph);
bool colorWithN(const std::vector<Web> &webs, const Graph<int> &graph, int colors, std::map<int, int> &assignment);
bool colorWithN(const std::vector<Web> &webs, const Graph<int> &graph, int colors, const std::set<int> &excludedWebs, std::map<int, int> &assignment);
ParsedInput loadInputData(const std::string &rangesPath, const std::string &registersPath);
AllocationResult runAllocation(const std::string &rangesPath, const std::string &registersPath);

std::string pointToString(const ProgramPoint &point);
std::string webPointsToString(const Web &web);
void printResult(std::ostream &out, const AllocationResult &result);
void printParsedInput(const ParsedInput &input);
void printDetailedResult(const AllocationResult &result);
void writeResultFile(const std::string &path, const AllocationResult &result);

#endif
