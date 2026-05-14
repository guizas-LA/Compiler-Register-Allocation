#include <iostream>
#include <string>
using namespace std;

#include "ui/BatchMode.h"
#include "ui/InteractiveMode.h"

int main(int argc, char *argv[]) {
    if (argc == 1) {
        interactiveMenu();
        return 0;
    }

    if (argc == 5 && string(argv[1]) == "-b") {
        return runBatch(argv[2], argv[3], argv[4]);
    }

    cerr << "Usage:\n";
    cerr << "  " << argv[0] << "\n";
    cerr << "  " << argv[0] << " -b ranges.txt registers.txt allocation.txt\n";
    return 1;
}
