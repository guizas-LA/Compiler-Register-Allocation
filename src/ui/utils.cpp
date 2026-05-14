#include "ui/Utils.h"
using namespace std;

#include <fstream>
#include <iostream>

#include "parser/parserTools.h"
#include "ui/Menu.h"

string askPath(const string &label) {
    cout << label;
    string path;
    cin >> path;
    return trim(path);
}

string resolvePath(const string &path, const vector<string> &searchDirs) {
    if (path.empty() || fileExists(path)) return path;
    if (path.find('/') != string::npos || path.find('\\') != string::npos) return path;

    for (const auto &dir : searchDirs) {
        string candidate = dir + "/" + path;
        if (fileExists(candidate)) return candidate;
    }

    return path;
}

void showFileContent(const string &path) {
    ifstream input(path);
    if (!input) {
        cerr << "Warning: could not open file: " << path << "\n";
        return;
    }

    string line;
    while (getline(input, line)) {
        string text = stripComment(line);
        if (text.empty()) continue;
        cout << text << "\n";
    }
}
