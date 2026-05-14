#include "ui/BatchMode.h"
using namespace std;

#include <exception>
#include <iostream>

#include "parser/Parser.h"

int runBatch(const string &rangesPath, const string &registersPath, const string &outputPath) {
    try {
        AllocationResult result = runAllocation(rangesPath, registersPath);
        writeResultFile(outputPath, result);

        if (!result.success) {
            cerr << "Warning: " << result.warning << "\n";
        }

        return 0;
    } catch (const exception &error) {
        cerr << "Error: " << error.what() << "\n";
        return 1;
    }
}
