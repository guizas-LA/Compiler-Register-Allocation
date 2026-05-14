#include "ui/InteractiveMode.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include "parser/Parser.h"
#include "parser/parserTools.h"
#include "ui/Menu.h"
#include "ui/Utils.h"

void interactiveMenu() {
    int option;
    string rangesPath;
    string registersPath;
    string outputPath;
    ParsedInput lastInput;
    bool hasInput = false;
    bool hasResult = false;
    AllocationResult lastResult;
    

    while (true) {

        showMenu();

        cin >> option;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cerr << "Error: invalid menu option\n";
            continue;
        }

        switch (option) {
                case 1:
                    rangesPath = askPath("Ranges file: ");
                    rangesPath = resolvePath(rangesPath, {"basic/ranges", "basic"});
                    if (!fileExists(rangesPath)) {
                        cerr << "Warning: file does not exist yet: " << rangesPath << "\n";
                    } else {
                        showFileContent(rangesPath);
                    }
                    break;

                case 2:
                    registersPath = askPath("Registers file: ");
                    registersPath = resolvePath(registersPath, {"basic/registers", "basic"});
                    if (!fileExists(registersPath)) {
                        cerr << "Warning: file does not exist yet: " << registersPath << "\n";
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
                    cout << "\nInput data loaded and validated successfully.\n";
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
                        cerr << "Warning: " << lastResult.warning << "\n";
                    }
                    break;

                case 6:
                    if (!hasResult) {
                        if (hasInput) {
                            printParsedInput(lastInput);
                        } else {
                            cout << "No input has been loaded and no allocation has been run yet.\n";
                        }
                    } else {
                        printDetailedResult(lastResult);
                    }
                    break;

                case 7:
                    if (!hasResult) {
                        cout << "No allocation has been run yet.\n";
                    } else {
                        if (outputPath.empty()) {
                            outputPath = askPath("Output file: ");
                            outputPath = resolvePath(outputPath, {"basic/output", "basic"});
                        }
                        writeResultFile(outputPath, lastResult);
                        cout << "Saved allocation to " << outputPath << "\n";
                    }
                    break;
                case 0:
                    return;
                default:
                    cerr << "Error: invalid menu option\n";
                    break;
            }
    }
}
