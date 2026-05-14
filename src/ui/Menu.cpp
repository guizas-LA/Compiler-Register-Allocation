#include "ui/Menu.h"

#include <iostream>

void showMenu() {
    std::cout << "\n=======================================\n";
    std::cout << "| Register Allocation Assignment Tool |\n";
    std::cout << "=======================================\n";
    std::cout << "| 1 - Set ranges file                 |\n";
    std::cout << "| 2 - Set registers file              |\n";
    std::cout << "| 3 - Set output file                 |\n";
    std::cout << "| 4 - Load and validate input data    |\n";
    std::cout << "| 5 - Run allocation                  |\n";
    std::cout << "| 6 - Show last result                |\n";
    std::cout << "| 7 - Save last result                |\n";
    std::cout << "| 0 - Exit                            |\n";
    std::cout << "=======================================\n";
    std::cout << "\nChoice: ";
}
