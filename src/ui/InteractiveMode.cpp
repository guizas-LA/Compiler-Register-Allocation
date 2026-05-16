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
    bool hasResultW = false;
    bool hasResultA = false;
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
                    if (rangesPath.empty()) {
                        rangesPath = askPath("Ranges file: ");
                        rangesPath = resolvePath(rangesPath, {"basic/ranges", "basic"});
                    }
                    if (registersPath.empty()) {
                        registersPath = askPath("Registers file: ");
                        registersPath = resolvePath(registersPath, {"basic/registers", "basic"});
                    }
                    lastInput = loadInputData(rangesPath, registersPath);
                    hasResultW = true;
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
                    lastResult = runAllocation(rangesPath, registersPath);
                    hasResultA = true;
                    break;

                case 5:
                    if (!hasResultW || !hasResultA) {
                        cout << "Build webs & interference graph (option 3) and run allocation (option 4) first.\n";
                    } else {
                        printDetailedResult(lastResult);
                    }
                    break;

                case 6:
                    if (!hasResultA) {
                        cout << "Run allocation (option 4) first.\n";
                    } else {
                        if (outputPath.empty()) {
                            outputPath = askPath("Output file: ");
                            if (outputPath.find('/') == string::npos && outputPath.find('\\') == string::npos) {
                                outputPath = "basic/output/" + outputPath;
                            } else {
                                outputPath = resolvePath(outputPath, {"basic/output", "basic"});
                            }
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
