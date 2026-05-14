#include "ui/InteractiveMode.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "parser/Parser.h"
#include "parser/parserTools.h"
#include "ui/Menu.h"
#include "ui/Utils.h"

void interactiveMenu() {
    int option;
    std::string rangesPath;
    std::string registersPath;
    std::string outputPath;
    ParsedInput lastInput;
    bool hasInput = false;
    bool hasResult = false;
    AllocationResult lastResult;
    

    while (true) {

        showMenu();

        std::cin >> option;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Error: invalid menu option\n";
            continue;
        }

        switch (option) {
                case 1:
                    rangesPath = askPath("Ranges file: ");
                    rangesPath = resolvePath(rangesPath, {"basic/ranges", "basic"});
                    if (!fileExists(rangesPath)) {
                        std::cerr << "Warning: file does not exist yet: " << rangesPath << "\n";
                    } else {
                        showFileContent(rangesPath);
                    }
                    break;

                case 2:
                    registersPath = askPath("Registers file: ");
                    registersPath = resolvePath(registersPath, {"basic/registers", "basic"});
                    if (!fileExists(registersPath)) {
                        std::cerr << "Warning: file does not exist yet: " << registersPath << "\n";
                    } else {
                        showFileContent(registersPath);
                    }
                    break;

                case 3:
                    outputPath = askPath("Output file: ");
                    outputPath = resolvePath(outputPath, {"basic/output", "basic"});
                    break;

                case 4:
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
                    break;

                case 5:
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
                    break;

                case 6:
                    if (!hasResult) {
                        if (hasInput) {
                            printParsedInput(lastInput);
                        } else {
                            std::cout << "No input has been loaded and no allocation has been run yet.\n";
                        }
                    } else {
                        printDetailedResult(lastResult);
                    }
                    break;

                case 7:
                    if (!hasResult) {
                        std::cout << "No allocation has been run yet.\n";
                    } else {
                        if (outputPath.empty()) {
                            outputPath = askPath("Output file: ");
                            outputPath = resolvePath(outputPath, {"basic/output", "basic"});
                        }
                        writeResultFile(outputPath, lastResult);
                        std::cout << "Saved allocation to " << outputPath << "\n";
                    }
                    break;
                case 0:
                    return;
                default:
                    std::cerr << "Error: invalid menu option\n";
                    break;
            }
    }
}
