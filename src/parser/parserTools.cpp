#include "parser/Parser.h"
#include "parser/parserTools.h"
using namespace std;

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>

bool isDigits(const string &text) {
    return !text.empty() && all_of(text.begin(), text.end(), [](unsigned char c) {
        return isdigit(c);
    });
}

ProgramPoint parseProgramPoint(const string &token, int lineNumber) {
    if (token.empty()) {
        throw runtime_error("empty range token at line " + to_string(lineNumber));
    }

    ProgramPoint point;
    string number = token;
    char suffix = number.back();

    if (suffix == '+' || suffix == '-') {
        point.begins = suffix == '+';
        point.ends = suffix == '-';
        number.pop_back();
    }

    if (!isDigits(number)) {
        throw runtime_error("invalid program point '" + token + "' at line " + to_string(lineNumber));
    }

    point.line = stoi(number);
    if (point.line <= 0) {
        throw runtime_error("program points must be positive at line " + to_string(lineNumber));
    }

    return point;
}

bool rangesOverlap(const LiveRange &a, const LiveRange &b) {
    set<int> lines;
    for (const ProgramPoint &point : a.points) lines.insert(point.line);
    for (const ProgramPoint &point : b.points) {
        if (lines.count(point.line)) return true;
    }
    return false;
}

string trim(const string &value) {
    size_t first = 0;
    while (first < value.size() && isspace(static_cast<unsigned char>(value[first]))) first++;

    size_t last = value.size();
    while (last > first && isspace(static_cast<unsigned char>(value[last - 1]))) last--;

    return value.substr(first, last - first);
}

string stripComment(const string &line) {
    size_t comment = line.find('#');
    return trim(comment == string::npos ? line : line.substr(0, comment));
}

vector<string> split(const string &text, char delimiter) {
    vector<string> parts;
    string part;
    stringstream ss(text);

    while (getline(ss, part, delimiter)) {
        parts.push_back(trim(part));
    }

    return parts;
}

bool fileExists(const string &path) {
    ifstream input(path);
    return input.good();
}
