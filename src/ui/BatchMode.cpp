#include "ui/BatchMode.h"

#include <exception>
#include <iostream>

#include "parser/Parser.h"

int runBatch(const std::string &rangesPath, const std::string &registersPath, const std::string &outputPath) {
    try {
        AllocationResult result = runAllocation(rangesPath, registersPath);
        writeResultFile(outputPath, result);

        if (!result.success) {
            std::cerr << "Warning: " << result.warning << "\n";
        }

        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }
}
