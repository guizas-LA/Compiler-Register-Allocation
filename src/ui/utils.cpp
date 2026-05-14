#include "ui/Utils.h"

#include <fstream>
#include <iostream>

#include "parser/parserTools.h"
#include "ui/Menu.h"

std::string askPath(const std::string &label) {
    std::cout << label;
    std::string path;
    std::cin >> path;
    return trim(path);
}

std::string resolvePath(const std::string &path, const std::vector<std::string> &searchDirs) {
    if (path.empty() || fileExists(path)) return path;
    if (path.find('/') != std::string::npos || path.find('\\') != std::string::npos) return path;

    for (const auto &dir : searchDirs) {
        std::string candidate = dir + "/" + path;
        if (fileExists(candidate)) return candidate;
    }

    return path;
}

void showFileContent(const std::string &path) {
    std::ifstream input(path);
    if (!input) {
        std::cerr << "Warning: could not open file: " << path << "\n";
        return;
    }

    std::string line;
    while (std::getline(input, line)) {
        std::string text = stripComment(line);
        if (text.empty()) continue;
        std::cout << text << "\n";
    }
}
