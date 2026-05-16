#include "ui/Menu.h"
using namespace std;

#include <iostream>

void showMenu() {
    cout << "\n=======================================\n";
    cout << "| Register Allocation Assignment Tool |\n";
    cout << "=======================================\n";
    cout << "| 1 - Set ranges file                 |\n";
    cout << "| 2 - Set registers file              |\n";
    cout << "| 3 - Build webs & interference graph |\n";
    cout << "| 4 - Run allocation                  |\n";
    cout << "| 5 - Display results                 |\n";
    cout << "| 6 - Export output file              |\n";
    cout << "| 0 - Exit                            |\n";
    cout << "=======================================\n";
    cout << "\nChoice: ";
}
