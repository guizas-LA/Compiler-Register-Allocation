#ifndef DA_PROJECT2_MENU_H
#define DA_PROJECT2_MENU_H

#include <string>
using namespace std;

/**
 * @brief Ask the user for a path in the interactive menu.
 * @complexity O(n), where n is the length of the entered path.
 */
string askPath(const string &label);

/**
 * @brief Display the interactive menu options.
 */
void showMenu();

#endif
