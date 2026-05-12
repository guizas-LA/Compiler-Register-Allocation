#ifndef DA_PROJECT2_BATCH_MODE_H
#define DA_PROJECT2_BATCH_MODE_H

#include <string>

/**
 * @brief Execute the required batch interface.
 * @complexity Same as runAllocation plus output writing.
 */
int runBatch(const std::string &rangesPath, const std::string &registersPath, const std::string &outputPath);

#endif
