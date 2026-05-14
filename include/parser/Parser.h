#ifndef DA_PROJECT2_PARSER_H
#define DA_PROJECT2_PARSER_H

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>
using namespace std;

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
    string variable;
    vector<ProgramPoint> points;
};

/**
 * @brief Register count and selected allocation algorithm read from settings.
 */
struct RegisterSettings {
    int registers = 0;
    string algorithm = "basic";
    int parameter = 0;
};

/**
 * @brief A live web created by merging overlapping live ranges of one variable.
 */
struct Web {
    int id = 0;
    string variable;
    vector<ProgramPoint> points;
    set<int> liveLines;
};

/**
 * @brief Parsed input data and derived structures used by later tasks.
 */
struct ParsedInput {
    vector<LiveRange> ranges;
    RegisterSettings settings;
    vector<Web> webs;
    Graph<int> interferenceGraph;
};

/**
 * @brief Final allocation result shown in the menu and written to output files.
 */
struct AllocationResult {
    bool success = false;
    int registersUsed = 0;
    vector<Web> webs;
    map<int, set<int>> interference;
    map<int, int> webToRegister;
    set<int> spilledWebs;
    string warning;
};

ProgramPoint parseProgramPoint(const string &token, int lineNumber);
vector<LiveRange> parseRangesFile(const string &path);
RegisterSettings parseRegistersFile(const string &path);
bool rangesOverlap(const LiveRange &a, const LiveRange &b);
vector<Web> buildWebs(const vector<LiveRange> &ranges);
void buildInterferenceGraph(const vector<Web> &webs, Graph<int> &graph);
map<int, set<int>> graphToInterferenceMap(const Graph<int> &graph);
bool colorWithN(const vector<Web> &webs, const Graph<int> &graph, int colors, map<int, int> &assignment);
bool colorWithN(const vector<Web> &webs, const Graph<int> &graph, int colors, const set<int> &excludedWebs, map<int, int> &assignment);
ParsedInput loadInputData(const string &rangesPath, const string &registersPath);
AllocationResult runAllocation(const string &rangesPath, const string &registersPath);

string pointToString(const ProgramPoint &point);
string webPointsToString(const Web &web);
void printResult(ostream &out, const AllocationResult &result);
void printParsedInput(const ParsedInput &input);
void printDetailedResult(const AllocationResult &result);
void writeResultFile(const string &path, const AllocationResult &result);

#endif
