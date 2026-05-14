#ifndef DA_PROJECT2_UTILS_H
#define DA_PROJECT2_UTILS_H

#include <string>
#include <vector>

std::string resolvePath(const std::string &path, const std::vector<std::string> &searchDirs);
void showFileContent(const std::string &path);

#endif
