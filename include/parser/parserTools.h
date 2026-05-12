#ifndef DA_PROJECT2_PARSER_TOOLS_H
#define DA_PROJECT2_PARSER_TOOLS_H

#include <string>
#include <vector>

/**
 * @brief Remove leading and trailing whitespace.
 * @complexity O(n), where n is the size of the input string.
 */
std::string trim(const std::string &value);

/**
 * @brief Remove inline comments beginning with '#'.
 * @complexity O(n), where n is the size of the input line.
 */
std::string stripComment(const std::string &line);

/**
 * @brief Split a string by a delimiter and trim all resulting tokens.
 * @complexity O(n), where n is the size of the input string.
 */
std::vector<std::string> split(const std::string &text, char delimiter);

/**
 * @brief Check whether a file can be opened for reading.
 * @complexity O(1) excluding filesystem overhead.
 */
bool fileExists(const std::string &path);

#endif
