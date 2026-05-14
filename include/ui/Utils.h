#ifndef DA_PROJECT2_UTILS_H
#define DA_PROJECT2_UTILS_H

#include <string>
#include <vector>
using namespace std;

string resolvePath(const string &path, const vector<string> &searchDirs);
void showFileContent(const string &path);

#endif
