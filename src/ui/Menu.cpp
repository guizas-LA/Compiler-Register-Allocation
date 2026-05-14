#include "ui/Menu.h"
using namespace std;

#include <iostream>

void showMenu() {
    cout << "\n=======================================\n";
    cout << "| Register Allocation Assignment Tool |\n";
    cout << "=======================================\n";
    cout << "| 1 - Set ranges file                 |\n";
    cout << "| 2 - Set registers file              |\n";
    cout << "| 3 - Set output file                 |\n";
    cout << "| 4 - Load and validate input data    |\n";
    cout << "| 5 - Run allocation                  |\n";
    cout << "| 6 - Show last result                |\n";
    cout << "| 7 - Save last result                |\n";
    cout << "| 0 - Exit                            |\n";
    cout << "=======================================\n";
    cout << "\nChoice: ";
}
