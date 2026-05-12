#include "ui/Menu.h"

#include <iostream>

#include "parser/parserTools.h"

std::string askPath(const std::string &label) {
    std::cout << label;
    std::string path;
    std::getline(std::cin, path);
    return trim(path);
}
