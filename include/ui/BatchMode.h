#ifndef DA_PROJECT2_BATCH_MODE_H
#define DA_PROJECT2_BATCH_MODE_H

#include <string>
using namespace std;

/**
 * @brief Execute the required batch interface.
 * @complexity Same as runAllocation plus output writing.
 */
int runBatch(const string &rangesPath, const string &registersPath, const string &outputPath);

#endif
