#include "ui/InteractiveMode.h"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "parser/Parser.h"
#include "parser/parserTools.h"
#include "ui/Menu.h"

static std::string resolvePath(const std::string &path, const std::vector<std::string> &searchDirs) {
    if (path.empty() || fileExists(path)) return path;
    if (path.find('/') != std::string::npos || path.find('\\') != std::string::npos) return path;

    for (const auto &dir : searchDirs) {
        std::string candidate = dir + "/" + path;
        if (fileExists(candidate)) return candidate;
    }

    return path;
}

void interactiveMenu() {
    std::string rangesPath;
    std::string registersPath;
    std::string outputPath;
    ParsedInput lastInput;
    bool hasInput = false;
    AllocationResult lastResult;
    bool hasResult = false;

    while (true) {
        std::cout << "=======================================\n";
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
        std::cout << "Choice: ";

        std::string choice;
        if (!std::getline(std::cin, choice)) {
            std::cout << "\nNo input available. Exiting interactive mode.\n";
            return;
        }
        choice = trim(choice);

        try {
            if (choice == "1") {
                rangesPath = askPath("Ranges file: ");
                rangesPath = resolvePath(rangesPath, {"basic/ranges", "basic"});
                if (!fileExists(rangesPath)) std::cerr << "Warning: file does not exist yet: " << rangesPath << "\n";
            } else if (choice == "2") {
                registersPath = askPath("Registers file: ");
                registersPath = resolvePath(registersPath, {"basic/registers", "basic"});
                if (!fileExists(registersPath)) std::cerr << "Warning: file does not exist yet: " << registersPath << "\n";
            } else if (choice == "3") {
                outputPath = askPath("Output file: ");
            } else if (choice == "4") {
                if (rangesPath.empty()) {
                    rangesPath = askPath("Ranges file: ");
                    rangesPath = resolvePath(rangesPath, {"basic/ranges", "basic"});
                }
                if (registersPath.empty()) {
                    registersPath = askPath("Registers file: ");
                    registersPath = resolvePath(registersPath, {"basic/registers", "basic"});
                }

                lastInput = loadInputData(rangesPath, registersPath);
                hasInput = true;
                printParsedInput(lastInput);
                std::cout << "\nInput data loaded and validated successfully.\n";
            } else if (choice == "5") {
                if (rangesPath.empty()) {
                    rangesPath = askPath("Ranges file: ");
                    rangesPath = resolvePath(rangesPath, {"basic/ranges", "basic"});
                }
                if (registersPath.empty()) {
                    registersPath = askPath("Registers file: ");
                    registersPath = resolvePath(registersPath, {"basic/registers", "basic"});
                }

                lastResult = runAllocation(rangesPath, registersPath);
                hasResult = true;
                printDetailedResult(lastResult);

                if (!lastResult.success) {
                    std::cerr << "Warning: " << lastResult.warning << "\n";
                }
            } else if (choice == "6") {
                if (!hasResult) {
                    if (hasInput) {
                        printParsedInput(lastInput);
                    } else {
                        std::cout << "No input has been loaded and no allocation has been run yet.\n";
                    }
                } else {
                    printDetailedResult(lastResult);
                }
            } else if (choice == "7") {
                if (!hasResult) {
                    std::cout << "No allocation has been run yet.\n";
                } else {
                    if (outputPath.empty()) outputPath = askPath("Output file: ");
                    writeResultFile(outputPath, lastResult);
                    std::cout << "Saved allocation to " << outputPath << "\n";
                }
            } else if (choice == "0") {
                return;
            } else {
                std::cerr << "Error: invalid menu option\n";
            }
        } catch (const std::exception &error) {
            std::cerr << "Error: " << error.what() << "\n";
        }
    }
}
