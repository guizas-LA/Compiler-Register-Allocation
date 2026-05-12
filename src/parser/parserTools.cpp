#include "parser/parserTools.h"

#include <cctype>
#include <fstream>
#include <sstream>

std::string trim(const std::string &value) {
    size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) first++;

    size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) last--;

    return value.substr(first, last - first);
}

std::string stripComment(const std::string &line) {
    size_t comment = line.find('#');
    return trim(comment == std::string::npos ? line : line.substr(0, comment));
}

std::vector<std::string> split(const std::string &text, char delimiter) {
    std::vector<std::string> parts;
    std::string part;
    std::stringstream ss(text);

    while (std::getline(ss, part, delimiter)) {
        parts.push_back(trim(part));
    }

    return parts;
}

bool fileExists(const std::string &path) {
    std::ifstream input(path);
    return input.good();
}
