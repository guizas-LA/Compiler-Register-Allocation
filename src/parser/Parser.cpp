#include "parser/Parser.h"

#include "parser/parserTools.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>

ProgramPoint parseProgramPoint(const std::string &token, int lineNumber) {
    if (token.empty()) {
        throw std::runtime_error("empty range token at line " + std::to_string(lineNumber));
    }

    ProgramPoint point;
    std::string number = token;
    char suffix = token.back();

    if (suffix == '+' || suffix == '-') {
        point.begins = suffix == '+';
        point.ends = suffix == '-';
        number = token.substr(0, token.size() - 1);
    }

    if (number.empty() || !std::all_of(number.begin(), number.end(), [](unsigned char c) { return std::isdigit(c); })) {
        throw std::runtime_error("invalid program point '" + token + "' at line " + std::to_string(lineNumber));
    }

    point.line = std::stoi(number);
    if (point.line <= 0) {
        throw std::runtime_error("program points must be positive at line " + std::to_string(lineNumber));
    }

    return point;
}

std::vector<LiveRange> parseRangesFile(const std::string &path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("could not open ranges file: " + path);
    }

    std::vector<LiveRange> ranges;
    std::map<std::string, std::pair<bool, bool>> variableMarkers;
    std::string line;
    int lineNumber = 0;

    while (std::getline(input, line)) {
        lineNumber++;
        line = stripComment(line);
        if (line.empty()) continue;

        size_t separator = line.find(':');
        if (separator == std::string::npos || line.find(':', separator + 1) != std::string::npos) {
            throw std::runtime_error("invalid range line " + std::to_string(lineNumber) + ": expected 'variable: points'");
        }

        LiveRange range;
        range.variable = trim(line.substr(0, separator));
        if (range.variable.empty()) {
            throw std::runtime_error("missing variable name at line " + std::to_string(lineNumber));
        }

        std::vector<std::string> tokens = split(line.substr(separator + 1), ',');
        if (tokens.empty()) {
            throw std::runtime_error("missing live range points at line " + std::to_string(lineNumber));
        }

        std::set<int> seenLines;
        for (const std::string &rangeToken : tokens) {
            ProgramPoint point = parseProgramPoint(rangeToken, lineNumber);
            if (!seenLines.insert(point.line).second) {
                throw std::runtime_error("duplicate program point " + std::to_string(point.line) + " at line " + std::to_string(lineNumber));
            }
            variableMarkers[range.variable].first = variableMarkers[range.variable].first || point.begins;
            variableMarkers[range.variable].second = variableMarkers[range.variable].second || point.ends;
            range.points.push_back(point);
        }

        ranges.push_back(range);
    }

    if (ranges.empty()) {
        throw std::runtime_error("ranges file has no live ranges");
    }

    for (const auto &entry : variableMarkers) {
        if (!entry.second.first || !entry.second.second) {
            throw std::runtime_error("variable '" + entry.first + "' must have at least one '+' and one '-' marker");
        }
    }

    return ranges;
}

RegisterSettings parseRegistersFile(const std::string &path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("could not open registers file: " + path);
    }

    RegisterSettings settings;
    bool sawRegisters = false;
    bool sawAlgorithm = false;
    std::string line;
    int lineNumber = 0;

    while (std::getline(input, line)) {
        lineNumber++;
        line = stripComment(line);
        if (line.empty()) continue;

        size_t separator = line.find(':');
        if (separator == std::string::npos) {
            throw std::runtime_error("invalid settings line " + std::to_string(lineNumber) + ": expected 'key: value'");
        }

        std::string key = trim(line.substr(0, separator));
        std::string value = trim(line.substr(separator + 1));

        if (key == "registers") {
            if (value.empty() || !std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); })) {
                throw std::runtime_error("invalid registers value at line " + std::to_string(lineNumber));
            }
            settings.registers = std::stoi(value);
            if (settings.registers <= 0) {
                throw std::runtime_error("register count must be positive");
            }
            sawRegisters = true;
        } else if (key == "algorithm") {
            std::vector<std::string> parts = split(value, ',');
            settings.algorithm = parts.empty() ? "" : parts[0];
            settings.parameter = 0;

            if (settings.algorithm != "basic" && settings.algorithm != "spilling" &&
                settings.algorithm != "splitting" && settings.algorithm != "free") {
                throw std::runtime_error("unknown algorithm '" + settings.algorithm + "'");
            }

            if (settings.algorithm == "spilling" || settings.algorithm == "splitting") {
                if (parts.size() != 2 || parts[1].empty() ||
                    !std::all_of(parts[1].begin(), parts[1].end(), [](unsigned char c) { return std::isdigit(c); })) {
                    throw std::runtime_error("algorithm '" + settings.algorithm + "' requires a numeric parameter");
                }
                settings.parameter = std::stoi(parts[1]);
                if (settings.parameter <= 0) {
                    throw std::runtime_error("algorithm parameter must be positive");
                }
            } else if (parts.size() != 1) {
                throw std::runtime_error("algorithm '" + settings.algorithm + "' does not accept a parameter");
            }

            sawAlgorithm = true;
        } else {
            throw std::runtime_error("unknown settings key '" + key + "' at line " + std::to_string(lineNumber));
        }
    }

    if (!sawRegisters) throw std::runtime_error("registers file is missing 'registers: N'");
    if (!sawAlgorithm) throw std::runtime_error("registers file is missing 'algorithm: ...'");

    return settings;
}

bool rangesOverlap(const LiveRange &a, const LiveRange &b) {
    std::set<int> lines;
    for (const ProgramPoint &point : a.points) lines.insert(point.line);
    for (const ProgramPoint &point : b.points) {
        if (lines.count(point.line)) return true;
    }
    return false;
}

std::vector<Web> buildWebs(const std::vector<LiveRange> &ranges) {
    std::vector<Web> webs;
    std::map<std::string, std::vector<LiveRange>> byVariable;

    for (const LiveRange &range : ranges) {
        byVariable[range.variable].push_back(range);
    }

    for (const auto &entry : byVariable) {
        const std::string &variable = entry.first;
        const std::vector<LiveRange> &variableRanges = entry.second;
        std::vector<int> disjointSetParent(variableRanges.size());
        for (size_t i = 0; i < disjointSetParent.size(); i++) disjointSetParent[i] = static_cast<int>(i);

        std::function<int(int)> findRoot = [&](int value) {
            while (disjointSetParent[value] != value) {
                disjointSetParent[value] = disjointSetParent[disjointSetParent[value]];
                value = disjointSetParent[value];
            }
            return value;
        };

        auto unite = [&](int a, int b) {
            int rootA = findRoot(a);
            int rootB = findRoot(b);
            if (rootA != rootB) disjointSetParent[rootB] = rootA;
        };

        for (size_t i = 0; i < variableRanges.size(); i++) {
            for (size_t j = i + 1; j < variableRanges.size(); j++) {
                if (rangesOverlap(variableRanges[i], variableRanges[j])) {
                    unite(static_cast<int>(i), static_cast<int>(j));
                }
            }
        }

        std::map<int, std::map<int, ProgramPoint>> merged;
        for (size_t i = 0; i < variableRanges.size(); i++) {
            int root = findRoot(static_cast<int>(i));
            for (const ProgramPoint &point : variableRanges[i].points) {
                ProgramPoint &stored = merged[root][point.line];
                stored.line = point.line;
                stored.begins = stored.begins || point.begins;
                stored.ends = stored.ends || point.ends;
            }
        }

        for (const auto &component : merged) {
            Web web;
            web.id = static_cast<int>(webs.size());
            web.variable = variable;

            for (const auto &pointEntry : component.second) {
                web.points.push_back(pointEntry.second);
                web.liveLines.insert(pointEntry.first);
            }

            webs.push_back(web);
        }
    }

    return webs;
}

void buildInterferenceGraph(const std::vector<Web> &webs, Graph<int> &graph) {
    for (const Web &web : webs) {
        graph.addVertex(web.id);
    }

    for (size_t i = 0; i < webs.size(); i++) {
        for (size_t j = i + 1; j < webs.size(); j++) {
            bool interferes = false;
            for (const ProgramPoint &left : webs[i].points) {
                for (const ProgramPoint &right : webs[j].points) {
                    if (left.line != right.line) continue;

                    bool leftEndsOnly = left.ends && !left.begins;
                    bool leftBeginsOnly = left.begins && !left.ends;
                    bool rightEndsOnly = right.ends && !right.begins;
                    bool rightBeginsOnly = right.begins && !right.ends;
                    bool onlyTouchesAtUseThenDefinition = (leftEndsOnly && rightBeginsOnly) || (rightEndsOnly && leftBeginsOnly);

                    if (!onlyTouchesAtUseThenDefinition) {
                        interferes = true;
                    }
                    break;
                }
                if (interferes) {
                    break;
                }
            }

            if (interferes) {
                graph.addEdge(webs[i].id, webs[j].id, 1);
                graph.addEdge(webs[j].id, webs[i].id, 1);
            }
        }
    }
}

std::map<int, std::set<int>> graphToInterferenceMap(const Graph<int> &graph) {
    std::map<int, std::set<int>> interference;

    for (Vertex<int> *vertex : graph.getVertexSet()) {
        int source = vertex->getInfo();
        interference[source];
        for (const Edge<int> &edge : vertex->getAdj()) {
            interference[source].insert(edge.getDest()->getInfo());
        }
    }

    return interference;
}

static int activeDegree(int webId, const std::map<int, std::set<int>> &interference, const std::set<int> &active) {
    int degree = 0;
    for (int neighbor : interference.at(webId)) {
        if (active.count(neighbor)) degree++;
    }
    return degree;
}

bool colorWithN(const std::vector<Web> &webs, const Graph<int> &graph, int colors, const std::set<int> &excludedWebs, std::map<int, int> &assignment) {
    std::map<int, std::set<int>> interference = graphToInterferenceMap(graph);
    std::set<int> active;
    std::vector<int> stack;

    for (const Web &web : webs) {
        if (!excludedWebs.count(web.id)) {
            active.insert(web.id);
        }
    }

    while (!active.empty()) {
        int selected = -1;
        int selectedDegree = -1;

        for (int webId : active) {
            int degree = activeDegree(webId, interference, active);
            if (degree < colors && (selected == -1 || degree < selectedDegree || (degree == selectedDegree && webId < selected))) {
                selected = webId;
                selectedDegree = degree;
            }
        }

        if (selected == -1) {
            assignment.clear();
            return false;
        }

        active.erase(selected);
        stack.push_back(selected);
    }

    assignment.clear();
    while (!stack.empty()) {
        int webId = stack.back();
        stack.pop_back();

        std::set<int> unavailable;
        for (int neighbor : interference.at(webId)) {
            if (assignment.count(neighbor)) unavailable.insert(assignment[neighbor]);
        }

        int chosen = -1;
        for (int color = 0; color < colors; color++) {
            if (!unavailable.count(color)) {
                chosen = color;
                break;
            }
        }

        if (chosen == -1) {
            assignment.clear();
            return false;
        }
        assignment[webId] = chosen;
    }

    return true;
}

bool colorWithN(const std::vector<Web> &webs, const Graph<int> &graph, int colors, std::map<int, int> &assignment) {
    return colorWithN(webs, graph, colors, std::set<int>(), assignment);
}

static std::vector<int> spillPriority(const std::vector<Web> &webs, const std::map<int, std::set<int>> &interference) {
    std::vector<int> candidates;
    for (const Web &web : webs) candidates.push_back(web.id);

    std::sort(candidates.begin(), candidates.end(), [&](int a, int b) {
        size_t degreeA = interference.at(a).size();
        size_t degreeB = interference.at(b).size();
        if (degreeA != degreeB) return degreeA > degreeB;

        size_t liveA = webs[a].liveLines.size();
        size_t liveB = webs[b].liveLines.size();
        if (liveA != liveB) return liveA > liveB;

        return a < b;
    });

    return candidates;
}

static bool nextCombination(std::vector<int> &indices, int n) {
    int k = static_cast<int>(indices.size());
    for (int i = k - 1; i >= 0; i--) {
        if (indices[i] < n - k + i) {
            indices[i]++;
            for (int j = i + 1; j < k; j++) {
                indices[j] = indices[j - 1] + 1;
            }
            return true;
        }
    }
    return false;
}

static bool tryColoringUpToLimit(const ParsedInput &input, const std::set<int> &excludedWebs, int &registersUsed, std::map<int, int> &assignment) {
    for (int colors = 1; colors <= input.settings.registers; colors++) {
        if (colorWithN(input.webs, input.interferenceGraph, colors, excludedWebs, assignment)) {
            registersUsed = colors;
            return true;
        }
    }
    return false;
}

static AllocationResult makeBaseResult(const ParsedInput &input) {
    AllocationResult result;
    result.webs = input.webs;
    result.interference = graphToInterferenceMap(input.interferenceGraph);
    return result;
}

static AllocationResult runBasicAllocation(const ParsedInput &input) {
    AllocationResult result = makeBaseResult(input);
    std::map<int, int> assignment;
    int registersUsed = 0;

    if (tryColoringUpToLimit(input, std::set<int>(), registersUsed, assignment)) {
        result.success = true;
        result.registersUsed = registersUsed;
        result.webToRegister = assignment;
        return result;
    }

    result.registersUsed = 0;
    for (const Web &web : result.webs) {
        result.spilledWebs.insert(web.id);
    }
    result.warning = "assignment to the provided number of registers was not possible";
    return result;
}

static AllocationResult runSpillingAllocation(const ParsedInput &input) {
    AllocationResult basicResult = runBasicAllocation(input);
    if (basicResult.success) {
        return basicResult;
    }

    AllocationResult result = makeBaseResult(input);
    std::vector<int> candidates = spillPriority(input.webs, result.interference);
    int maxSpills = std::min(input.settings.parameter, static_cast<int>(candidates.size()));

    for (int spillCount = 1; spillCount <= maxSpills; spillCount++) {
        std::vector<int> indices(spillCount);
        for (int i = 0; i < spillCount; i++) indices[i] = i;

        do {
            std::set<int> excludedWebs;
            for (int index : indices) {
                excludedWebs.insert(candidates[index]);
            }

            std::map<int, int> assignment;
            int registersUsed = 0;
            if (tryColoringUpToLimit(input, excludedWebs, registersUsed, assignment)) {
                result.success = true;
                result.registersUsed = registersUsed;
                result.webToRegister = assignment;
                result.spilledWebs = excludedWebs;
                result.warning = "web spilling used: selected highest-degree webs first to reduce interference";
                return result;
            }
        } while (nextCombination(indices, static_cast<int>(candidates.size())));
    }

    result.registersUsed = 0;
    for (const Web &web : result.webs) {
        result.spilledWebs.insert(web.id);
    }
    result.warning = "assignment to the provided number of registers was not possible, even with the allowed web spilling";
    return result;
}

static ParsedInput inputWithWebs(const ParsedInput &input, const std::vector<Web> &webs) {
    ParsedInput changed = input;
    changed.webs = webs;
    changed.interferenceGraph = Graph<int>();
    buildInterferenceGraph(changed.webs, changed.interferenceGraph);
    return changed;
}

static std::vector<Web> renumberWebs(std::vector<Web> webs) {
    for (size_t i = 0; i < webs.size(); i++) {
        webs[i].id = static_cast<int>(i);
    }
    return webs;
}

static bool splitSingleWeb(const Web &web, Web &first, Web &second) {
    if (web.points.size() < 2) {
        return false;
    }

    size_t splitPoint = web.points.size() / 2;
    if (splitPoint == 0 || splitPoint >= web.points.size()) {
        return false;
    }

    first = web;
    second = web;
    first.points.assign(web.points.begin(), web.points.begin() + static_cast<long>(splitPoint));
    second.points.assign(web.points.begin() + static_cast<long>(splitPoint), web.points.end());

    first.points.back().ends = true;
    second.points.front().begins = true;

    first.liveLines.clear();
    second.liveLines.clear();
    for (const ProgramPoint &point : first.points) first.liveLines.insert(point.line);
    for (const ProgramPoint &point : second.points) second.liveLines.insert(point.line);

    return true;
}

static std::vector<Web> splitSelectedWebs(const std::vector<Web> &webs, const std::set<int> &selectedWebs) {
    std::vector<Web> result;

    for (const Web &web : webs) {
        if (!selectedWebs.count(web.id)) {
            result.push_back(web);
            continue;
        }

        Web first;
        Web second;
        if (splitSingleWeb(web, first, second)) {
            result.push_back(first);
            result.push_back(second);
        } else {
            result.push_back(web);
        }
    }

    return renumberWebs(result);
}

static AllocationResult runSplittingAllocation(const ParsedInput &input) {
    AllocationResult basicResult = runBasicAllocation(input);
    if (basicResult.success) {
        return basicResult;
    }

    std::map<int, std::set<int>> interference = graphToInterferenceMap(input.interferenceGraph);
    std::vector<int> candidates = spillPriority(input.webs, interference);
    candidates.erase(
        std::remove_if(candidates.begin(), candidates.end(), [&](int webId) {
            return input.webs[webId].points.size() < 2;
        }),
        candidates.end()
    );

    int maxSplits = std::min(input.settings.parameter, static_cast<int>(candidates.size()));
    for (int splitCount = 1; splitCount <= maxSplits; splitCount++) {
        std::vector<int> indices(splitCount);
        for (int i = 0; i < splitCount; i++) indices[i] = i;

        do {
            std::set<int> selectedWebs;
            for (int index : indices) {
                selectedWebs.insert(candidates[index]);
            }

            std::vector<Web> splitWebs = splitSelectedWebs(input.webs, selectedWebs);
            ParsedInput changed = inputWithWebs(input, splitWebs);
            AllocationResult result = runBasicAllocation(changed);
            if (result.success) {
                result.warning = "web splitting used: selected highest-degree, longest live webs first";
                return result;
            }
        } while (nextCombination(indices, static_cast<int>(candidates.size())));
    }

    AllocationResult result = makeBaseResult(input);
    result.registersUsed = 0;
    for (const Web &web : result.webs) {
        result.spilledWebs.insert(web.id);
    }
    result.warning = "assignment to the provided number of registers was not possible, even with the allowed web splitting";
    return result;
}

ParsedInput loadInputData(const std::string &rangesPath, const std::string &registersPath) {
    ParsedInput input;
    input.ranges = parseRangesFile(rangesPath);
    input.settings = parseRegistersFile(registersPath);
    input.webs = buildWebs(input.ranges);
    buildInterferenceGraph(input.webs, input.interferenceGraph);
    return input;
}

AllocationResult runAllocation(const std::string &rangesPath, const std::string &registersPath) {
    ParsedInput input = loadInputData(rangesPath, registersPath);

    if (input.settings.algorithm == "basic") {
        return runBasicAllocation(input);
    }

    if (input.settings.algorithm == "spilling") {
        return runSpillingAllocation(input);
    }

    if (input.settings.algorithm == "splitting") {
        return runSplittingAllocation(input);
    }

    throw std::runtime_error("algorithm '" + input.settings.algorithm + "' is parsed but not implemented yet");
}

std::string pointToString(const ProgramPoint &point) {
    std::string text = std::to_string(point.line);
    if (point.begins) text += "+";
    if (point.ends) text += "-";
    return text;
}

std::string webPointsToString(const Web &web) {
    std::stringstream ss;
    for (size_t i = 0; i < web.points.size(); i++) {
        if (i > 0) ss << ",";
        ss << pointToString(web.points[i]);
    }
    return ss.str();
}

void printResult(std::ostream &out, const AllocationResult &result) {
    out << "webs: " << result.webs.size() << "\n";
    for (const Web &web : result.webs) {
        out << "web" << web.id << ": " << webPointsToString(web) << "\n";
    }

    out << "\nregisters: " << result.registersUsed << "\n";
    if (result.success) {
        for (const auto &entry : result.webToRegister) {
            out << "r" << entry.second << ": web" << entry.first << "\n";
        }
        for (int webId : result.spilledWebs) {
            out << "M: web" << webId << "\n";
        }
    } else {
        for (const Web &web : result.webs) {
            out << "M: web" << web.id << "\n";
        }
    }
}

void printParsedInput(const ParsedInput &input) {
    std::cout << "\nParsed ranges: " << input.ranges.size() << "\n";
    for (const LiveRange &range : input.ranges) {
        std::cout << "  " << range.variable << ": ";
        for (size_t i = 0; i < range.points.size(); i++) {
            if (i > 0) std::cout << ",";
            std::cout << pointToString(range.points[i]);
        }
        std::cout << "\n";
    }

    std::cout << "\nRegister settings\n";
    std::cout << "  registers: " << input.settings.registers << "\n";
    std::cout << "  algorithm: " << input.settings.algorithm;
    if (input.settings.parameter > 0) std::cout << ", " << input.settings.parameter;
    std::cout << "\n";

    std::cout << "\nDerived webs\n";
    for (const Web &web : input.webs) {
        std::cout << "  web" << web.id << " (" << web.variable << "): " << webPointsToString(web) << "\n";
    }

    std::cout << "\nInterference graph\n";
    std::map<int, std::set<int>> interference = graphToInterferenceMap(input.interferenceGraph);
    for (const auto &entry : interference) {
        std::cout << "  web" << entry.first << " ->";
        if (entry.second.empty()) {
            std::cout << " none";
        } else {
            for (int neighbor : entry.second) std::cout << " web" << neighbor;
        }
        std::cout << "\n";
    }
}

void printDetailedResult(const AllocationResult &result) {
    std::cout << "\nWebs\n";
    for (const Web &web : result.webs) {
        std::cout << "  web" << web.id << " (" << web.variable << "): " << webPointsToString(web) << "\n";
    }

    std::cout << "\nInterference graph\n";
    for (const auto &entry : result.interference) {
        std::cout << "  web" << entry.first << " ->";
        if (entry.second.empty()) {
            std::cout << " none";
        } else {
            for (int neighbor : entry.second) std::cout << " web" << neighbor;
        }
        std::cout << "\n";
    }

    std::cout << "\nRegister allocation\n";
    printResult(std::cout, result);

    if (!result.spilledWebs.empty() && result.success) {
        std::cout << "\nSpilling rationale\n";
        std::cout << "  Spilled webs are selected by descending interference degree, then by longer live range.\n";
    }

    if (!result.warning.empty()) {
        std::cout << "\nWarning: " << result.warning << "\n";
    }
}

void writeResultFile(const std::string &path, const AllocationResult &result) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("could not open output file: " + path);
    }
    printResult(output, result);
}
