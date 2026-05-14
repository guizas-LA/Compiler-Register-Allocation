#ifndef DA_PROJECT2_PARSER_TOOLS_H
#define DA_PROJECT2_PARSER_TOOLS_H

#include <string>
#include <vector>
using namespace std;

/**
 * @brief Remove leading and trailing whitespace.
 * @complexity O(n), where n is the size of the input string.
 */
string trim(const string &value);

/**
 * @brief Remove inline comments beginning with '#'.
 * @complexity O(n), where n is the size of the input line.
 */
string stripComment(const string &line);

/**
 * @brief Split a string by a delimiter and trim all resulting tokens.
 * @complexity O(n), where n is the size of the input string.
 */
vector<string> split(const string &text, char delimiter);

/**
 * @brief Check whether a file can be opened for reading.
 * @complexity O(1) excluding filesystem overhead.
 */
bool fileExists(const string &path);

bool isDigits(const string &text);

#endif
