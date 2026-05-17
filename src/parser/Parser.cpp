#include "parser/Parser.h"
using namespace std;

#include "parser/parserTools.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>

vector<LiveRange> parseRangesFile(const string &path) {
    ifstream input(path);
    if (!input) {
        throw runtime_error("could not open ranges file: " + path);
    }

    vector<LiveRange> ranges;
    map<string, pair<bool, bool>> variableMarkers;
    string line;
    int lineNumber = 0;

    while (getline(input, line)) {
        lineNumber++;
        line = stripComment(line);
        if (line.empty()) continue;

        size_t separator = line.find(':');
        if (separator == string::npos || line.find(':', separator + 1) != string::npos) {
            throw runtime_error("invalid range line " + to_string(lineNumber) + ": expected 'variable: points'");
        }

        LiveRange range;
        range.variable = trim(line.substr(0, separator));
        if (range.variable.empty()) {
            throw runtime_error("missing variable name at line " + to_string(lineNumber));
        }

        vector<string> tokens = split(line.substr(separator + 1), ',');
        if (tokens.empty()) {
            throw runtime_error("missing live range points at line " + to_string(lineNumber));
        }

        set<int> seenLines;
        for (const auto &rangeToken : tokens) {
            ProgramPoint point = parseProgramPoint(rangeToken, lineNumber);
            if (!seenLines.insert(point.line).second) {
                throw runtime_error("duplicate program point " + to_string(point.line) + " at line " + to_string(lineNumber));
            }
            auto &marks = variableMarkers[range.variable];
            marks.first = marks.first || point.begins;
            marks.second = marks.second || point.ends;
            range.points.push_back(point);
        }

        ranges.push_back(range);
    }

    if (ranges.empty()) {
        throw runtime_error("ranges file has no live ranges");
    }

    for (const auto &entry : variableMarkers) {
        if (!entry.second.first || !entry.second.second) {
            throw runtime_error("variable '" + entry.first + "' must have at least one '+' and one '-' marker");
        }
    }

    return ranges;
}

RegisterSettings parseRegistersFile(const string &path) {
    ifstream input(path);
    if (!input) {
        throw runtime_error("could not open registers file: " + path);
    }

    auto parsePositiveInt = [&](const string &text, const string &message) {
        if (!isDigits(text)) {
            throw runtime_error(message);
        }
        int value = stoi(text);
        if (value <= 0) {
            throw runtime_error(message);
        }
        return value;
    };

    RegisterSettings settings;
    bool sawRegisters = false;
    bool sawAlgorithm = false;
    string line;
    int lineNumber = 0;

    while (getline(input, line)) {
        lineNumber++;
        line = stripComment(line);
        if (line.empty()) continue;

        size_t separator = line.find(':');
        if (separator == string::npos) {
            throw runtime_error("invalid settings line " + to_string(lineNumber) + ": expected 'key: value'");
        }

        string key = trim(line.substr(0, separator));
        string value = trim(line.substr(separator + 1));

        if (key == "registers") {
            settings.registers = parsePositiveInt(value, "invalid registers value at line " + to_string(lineNumber));
            sawRegisters = true;
        } else if (key == "algorithm") {
            vector<string> parts = split(value, ',');
            if (parts.empty()) {
                throw runtime_error("invalid algorithm line at line " + to_string(lineNumber));
            }

            settings.algorithm = parts[0];
            settings.parameter = 0;

            if (settings.algorithm != "basic" && settings.algorithm != "spilling" &&
                settings.algorithm != "splitting" && settings.algorithm != "free") {
                throw runtime_error("unknown algorithm '" + settings.algorithm + "'");
            }

            if (settings.algorithm == "spilling" || settings.algorithm == "splitting") {
                if (parts.size() != 2) {
                    throw runtime_error("algorithm '" + settings.algorithm + "' requires a numeric parameter");
                }
                settings.parameter = parsePositiveInt(parts[1], "algorithm '" + settings.algorithm + "' requires a numeric parameter");
            } else if (parts.size() != 1) {
                throw runtime_error("algorithm '" + settings.algorithm + "' does not accept a parameter");
            }

            sawAlgorithm = true;
        } else {
            throw runtime_error("unknown settings key '" + key + "' at line " + to_string(lineNumber));
        }
    }

    if (!sawRegisters) throw runtime_error("registers file is missing 'registers: N'");
    if (!sawAlgorithm) throw runtime_error("registers file is missing 'algorithm: ...'");

    return settings;
}

vector<Web> buildWebs(const vector<LiveRange> &ranges) {
    vector<Web> webs;
    map<string, vector<LiveRange>> byVariable;
    for (const auto &range : ranges) {
        byVariable[range.variable].push_back(range);
    }

    for (auto &entry : byVariable) {
        const string &variable = entry.first;
        vector<LiveRange> &variableRanges = entry.second;
        int n = static_cast<int>(variableRanges.size());
        vector<int> dsuParent(n);
        for (int i = 0; i < n; i++) dsuParent[i] = i;

        auto findRoot = [&](int value) -> int {
            while (dsuParent[value] != value) {
                dsuParent[value] = dsuParent[dsuParent[value]];
                value = dsuParent[value];
            }
            return value;
        };

        auto unite = [&](int a, int b) {
            int rootA = findRoot(a);
            int rootB = findRoot(b);
            if (rootA != rootB) dsuParent[rootB] = rootA;
        };

        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                if (rangesOverlap(variableRanges[i], variableRanges[j])) {
                    unite(i, j);
                }
            }
        }

        map<int, map<int, ProgramPoint>> merged;
        for (int i = 0; i < n; i++) {
            int root = findRoot(i);
            for (const auto &point : variableRanges[i].points) {
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

void buildInterferenceGraph(const vector<Web> &webs, Graph<int> &graph) {
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

map<int, set<int>> graphToInterferenceMap(const Graph<int> &graph) {
    map<int, set<int>> interference;

    for (Vertex<int> *vertex : graph.getVertexSet()) {
        int source = vertex->getInfo();
        interference[source];
        for (const Edge<int> &edge : vertex->getAdj()) {
            interference[source].insert(edge.getDest()->getInfo());
        }
    }

    return interference;
}

ParsedInput loadInputData(const string &rangesPath, const string &registersPath) {
    ParsedInput input;
    input.ranges = parseRangesFile(rangesPath);
    input.settings = parseRegistersFile(registersPath);
    input.webs = buildWebs(input.ranges);
    buildInterferenceGraph(input.webs, input.interferenceGraph);
    return input;
}

string pointToString(const ProgramPoint &point) {
    string text = to_string(point.line);
    if (point.begins) text += "+";
    if (point.ends) text += "-";
    return text;
}

string webPointsToString(const Web &web) {
    stringstream ss;
    for (size_t i = 0; i < web.points.size(); i++) {
        if (i > 0) ss << ",";
        ss << pointToString(web.points[i]);
    }
    return ss.str();
}

void printResult(ostream &out, const AllocationResult &result) {
    out << "# Total number of webs followed by the listing of the program points of each one\n";
    out << "# program points in each web are sorted in ascending order\n";
    out << "webs: " << result.webs.size() << "\n";
    for (const Web &web : result.webs) {
        out << "web" << web.id << ": " << webPointsToString(web) << "\n";
    }

    out << "# Total number of registers used, followed by assignment to webs\n";
    out << "registers: " << result.registersUsed << "\n";
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
    cout << "\nParsed ranges: " << input.ranges.size() << "\n";
    for (const LiveRange &range : input.ranges) {
        cout << "  " << range.variable << ": ";
        for (size_t i = 0; i < range.points.size(); i++) {
            if (i > 0) cout << ",";
            cout << pointToString(range.points[i]);
        }
        cout << "\n";
    }

    cout << "\nRegister settings\n";
    cout << "  registers: " << input.settings.registers << "\n";
    cout << "  algorithm: " << input.settings.algorithm;
    if (input.settings.parameter > 0) cout << ", " << input.settings.parameter;
    cout << "\n";

    cout << "\nDerived webs\n";
    for (const Web &web : input.webs) {
        cout << "  web" << web.id << " (" << web.variable << "): " << webPointsToString(web) << "\n";
    }

    cout << "\nInterference graph\n";
    map<int, set<int>> interference = graphToInterferenceMap(input.interferenceGraph);
    for (const auto &entry : interference) {
        cout << "  web" << entry.first << " ->";
        if (entry.second.empty()) {
            cout << " none";
        } else {
            for (int neighbor : entry.second) cout << " web" << neighbor;
        }
        cout << "\n";
    }
}

void printDetailedResult(const AllocationResult &result) {
    cout << "\nWebs\n";
    for (const Web &web : result.webs) {
        cout << "  web" << web.id << " (" << web.variable << "): " << webPointsToString(web) << "\n";
    }

    cout << "\nInterference graph\n";
    for (const auto &entry : result.interference) {
        cout << "  web" << entry.first << " ->";
        if (entry.second.empty()) {
            cout << " none";
        } else {
            for (int neighbor : entry.second) cout << " web" << neighbor;
        }
        cout << "\n";
    }

    cout << "\nRegister allocation\n\n";
    printResult(cout, result);

    if (!result.spilledWebs.empty() && result.success) {
        cout << "\nSpilling rationale\n";
        cout << "  Spilled webs are selected by descending interference degree, then by longer live range.\n";
    }

    if (!result.warning.empty()) {
        cout << "\n" << (result.success ? "Info: " : "Warning: ") << result.warning << "\n";
    }
}

void writeResultFile(const string &path, const AllocationResult &result) {
    ofstream output(path);
    if (!output) {
        throw runtime_error("could not open output file: " + path);
    }
    printResult(output, result);
}
