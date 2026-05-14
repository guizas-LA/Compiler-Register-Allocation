#include "parser/Parser.h"
using namespace std;

#include <algorithm>
#include <stdexcept>

static int activeDegree(int webId, const map<int, set<int>> &interference, const set<int> &active) {
    int degree = 0;
    for (int neighbor : interference.at(webId)) {
        if (active.count(neighbor)) degree++;
    }
    return degree;
}

bool colorWithN(const vector<Web> &webs, const Graph<int> &graph, int colors, const set<int> &excludedWebs, map<int, int> &assignment) {
    map<int, set<int>> interference = graphToInterferenceMap(graph);
    set<int> active;
    vector<int> stack;

    for (const Web &web : webs) {
        if (!excludedWebs.count(web.id)) {
            active.insert(web.id);
        }
    }

    while (!active.empty()) {
        int selected = -1;
        int selectedDegree = -1;

        for (int webId : active) {
            int degree = activeDegree(webId, interference, active);
            if (degree < colors && (selected == -1 || degree < selectedDegree || (degree == selectedDegree && webId < selected))) {
                selected = webId;
                selectedDegree = degree;
            }
        }

        if (selected == -1) {
            assignment.clear();
            return false;
        }

        active.erase(selected);
        stack.push_back(selected);
    }

    assignment.clear();
    while (!stack.empty()) {
        int webId = stack.back();
        stack.pop_back();

        set<int> unavailable;
        for (int neighbor : interference.at(webId)) {
            if (assignment.count(neighbor)) unavailable.insert(assignment[neighbor]);
        }

        int chosen = -1;
        for (int color = 0; color < colors; color++) {
            if (!unavailable.count(color)) {
                chosen = color;
                break;
            }
        }

        if (chosen == -1) {
            assignment.clear();
            return false;
        }
        assignment[webId] = chosen;
    }

    return true;
}

bool colorWithN(const vector<Web> &webs, const Graph<int> &graph, int colors, map<int, int> &assignment) {
    return colorWithN(webs, graph, colors, set<int>(), assignment);
}

static bool tryColoringUpToLimit(const ParsedInput &input, const set<int> &excludedWebs, int &registersUsed, map<int, int> &assignment) {
    for (int colors = 1; colors <= input.settings.registers; colors++) {
        if (colorWithN(input.webs, input.interferenceGraph, colors, excludedWebs, assignment)) {
            registersUsed = colors;
            return true;
        }
    }
    return false;
}

static AllocationResult makeBaseResult(const ParsedInput &input) {
    AllocationResult result;
    result.webs = input.webs;
    result.interference = graphToInterferenceMap(input.interferenceGraph);
    return result;
}

static AllocationResult runBasicAllocation(const ParsedInput &input) {
    AllocationResult result = makeBaseResult(input);
    map<int, int> assignment;
    int registersUsed = 0;

    if (tryColoringUpToLimit(input, set<int>(), registersUsed, assignment)) {
        result.success = true;
        result.registersUsed = registersUsed;
        result.webToRegister = assignment;
        return result;
    }

    result.registersUsed = 0;
    for (const Web &web : result.webs) {
        result.spilledWebs.insert(web.id);
    }
    result.warning = "assignment to the provided number of registers was not possible";
    return result;
}

static vector<int> spillPriority(const vector<Web> &webs, const map<int, set<int>> &interference) {
    vector<int> candidates;
    for (const Web &web : webs) candidates.push_back(web.id);

    sort(candidates.begin(), candidates.end(), [&](int a, int b) {
        size_t degreeA = interference.at(a).size();
        size_t degreeB = interference.at(b).size();
        if (degreeA != degreeB) return degreeA > degreeB;

        size_t liveA = webs[a].liveLines.size();
        size_t liveB = webs[b].liveLines.size();
        if (liveA != liveB) return liveA > liveB;

        return a < b;
    });

    return candidates;
}

static bool nextCombination(vector<int> &indices, int n) {
    int k = static_cast<int>(indices.size());
    for (int i = k - 1; i >= 0; i--) {
        if (indices[i] < n - k + i) {
            indices[i]++;
            for (int j = i + 1; j < k; j++) {
                indices[j] = indices[j - 1] + 1;
            }
            return true;
        }
    }
    return false;
}

static AllocationResult runSpillingAllocation(const ParsedInput &input) {
    AllocationResult basicResult = runBasicAllocation(input);
    if (basicResult.success) {
        return basicResult;
    }

    AllocationResult result = makeBaseResult(input);
    vector<int> candidates = spillPriority(input.webs, result.interference);
    int maxSpills = min(input.settings.parameter, static_cast<int>(candidates.size()));

    for (int spillCount = 1; spillCount <= maxSpills; spillCount++) {
        vector<int> indices(spillCount);
        for (int i = 0; i < spillCount; i++) indices[i] = i;

        do {
            set<int> excludedWebs;
            for (int index : indices) {
                excludedWebs.insert(candidates[index]);
            }

            map<int, int> assignment;
            int registersUsed = 0;
            if (tryColoringUpToLimit(input, excludedWebs, registersUsed, assignment)) {
                result.success = true;
                result.registersUsed = registersUsed;
                result.webToRegister = assignment;
                result.spilledWebs = excludedWebs;
                result.warning = "web spilling used: selected highest-degree webs first to reduce interference";
                return result;
            }
        } while (nextCombination(indices, static_cast<int>(candidates.size())));
    }

    result.registersUsed = 0;
    for (const Web &web : result.webs) {
        result.spilledWebs.insert(web.id);
    }
    result.warning = "assignment to the provided number of registers was not possible, even with the allowed web spilling";
    return result;
}

static ParsedInput inputWithWebs(const ParsedInput &input, const vector<Web> &webs) {
    ParsedInput changed = input;
    changed.webs = webs;
    changed.interferenceGraph = Graph<int>();
    buildInterferenceGraph(changed.webs, changed.interferenceGraph);
    return changed;
}

static vector<Web> renumberWebs(vector<Web> webs) {
    for (size_t i = 0; i < webs.size(); i++) {
        webs[i].id = static_cast<int>(i);
    }
    return webs;
}

static bool splitSingleWeb(const Web &web, Web &first, Web &second) {
    if (web.points.size() < 2) {
        return false;
    }

    size_t splitPoint = web.points.size() / 2;
    if (splitPoint == 0 || splitPoint >= web.points.size()) {
        return false;
    }

    first = web;
    second = web;
    first.points.assign(web.points.begin(), web.points.begin() + static_cast<long>(splitPoint));
    second.points.assign(web.points.begin() + static_cast<long>(splitPoint), web.points.end());

    first.points.back().ends = true;
    second.points.front().begins = true;

    first.liveLines.clear();
    second.liveLines.clear();
    for (const ProgramPoint &point : first.points) first.liveLines.insert(point.line);
    for (const ProgramPoint &point : second.points) second.liveLines.insert(point.line);

    return true;
}

static vector<Web> splitSelectedWebs(const vector<Web> &webs, const set<int> &selectedWebs) {
    vector<Web> result;

    for (const Web &web : webs) {
        if (!selectedWebs.count(web.id)) {
            result.push_back(web);
            continue;
        }

        Web first;
        Web second;
        if (splitSingleWeb(web, first, second)) {
            result.push_back(first);
            result.push_back(second);
        } else {
            result.push_back(web);
        }
    }

    return renumberWebs(result);
}

static AllocationResult runSplittingAllocation(const ParsedInput &input) {
    AllocationResult basicResult = runBasicAllocation(input);
    if (basicResult.success) {
        return basicResult;
    }

    map<int, set<int>> interference = graphToInterferenceMap(input.interferenceGraph);
    vector<int> candidates = spillPriority(input.webs, interference);
    candidates.erase(
        remove_if(candidates.begin(), candidates.end(), [&](int webId) {
            return input.webs[webId].points.size() < 2;
        }),
        candidates.end()
    );

    int maxSplits = min(input.settings.parameter, static_cast<int>(candidates.size()));
    for (int splitCount = 1; splitCount <= maxSplits; splitCount++) {
        vector<int> indices(splitCount);
        for (int i = 0; i < splitCount; i++) indices[i] = i;

        do {
            set<int> selectedWebs;
            for (int index : indices) {
                selectedWebs.insert(candidates[index]);
            }

            vector<Web> splitWebs = splitSelectedWebs(input.webs, selectedWebs);
            ParsedInput changed = inputWithWebs(input, splitWebs);
            AllocationResult result = runBasicAllocation(changed);
            if (result.success) {
                result.warning = "web splitting used: selected highest-degree, longest live webs first";
                return result;
            }
        } while (nextCombination(indices, static_cast<int>(candidates.size())));
    }

    AllocationResult result = makeBaseResult(input);
    result.registersUsed = 0;
    for (const Web &web : result.webs) {
        result.spilledWebs.insert(web.id);
    }
    result.warning = "assignment to the provided number of registers was not possible, even with the allowed web splitting";
    return result;
}

static int saturationDegree(int webId, const map<int, set<int>> &interference, const map<int, int> &assignment) {
    set<int> neighborColors;
    for (int neighbor : interference.at(webId)) {
        auto it = assignment.find(neighbor);
        if (it != assignment.end()) {
            neighborColors.insert(it->second);
        }
    }
    return static_cast<int>(neighborColors.size());
}

static int chooseDsaturVertex(const set<int> &active, const map<int, set<int>> &interference, const map<int, int> &assignment) {
    int selected = -1;
    int bestSaturation = -1;
    int bestDegree = -1;

    for (int webId : active) {
        if (assignment.count(webId)) continue;

        int saturation = saturationDegree(webId, interference, assignment);
        int degree = activeDegree(webId, interference, active);

        if (selected == -1 || saturation > bestSaturation ||
            (saturation == bestSaturation && degree > bestDegree) ||
            (saturation == bestSaturation && degree == bestDegree && webId < selected)) {
            selected = webId;
            bestSaturation = saturation;
            bestDegree = degree;
        }
    }

    return selected;
}

static bool dsaturBacktrack(const set<int> &active, const map<int, set<int>> &interference, int colors, map<int, int> &assignment) {
    if (assignment.size() == active.size()) {
        return true;
    }

    int webId = chooseDsaturVertex(active, interference, assignment);
    if (webId == -1) {
        return true;
    }

    set<int> unavailable;
    for (int neighbor : interference.at(webId)) {
        auto it = assignment.find(neighbor);
        if (it != assignment.end()) {
            unavailable.insert(it->second);
        }
    }

    for (int color = 0; color < colors; color++) {
        if (unavailable.count(color)) continue;

        assignment[webId] = color;
        if (dsaturBacktrack(active, interference, colors, assignment)) {
            return true;
        }
        assignment.erase(webId);
    }

    return false;
}

static bool dsaturColorWithN(const vector<Web> &webs, const Graph<int> &graph, int colors, const set<int> &excludedWebs, map<int, int> &assignment) {
    map<int, set<int>> interference = graphToInterferenceMap(graph);
    set<int> active;
    for (const Web &web : webs) {
        if (!excludedWebs.count(web.id)) {
            active.insert(web.id);
        }
    }

    assignment.clear();
    return dsaturBacktrack(active, interference, colors, assignment);
}

static bool tryDsaturUpToLimit(const ParsedInput &input, const set<int> &excludedWebs, int &registersUsed, map<int, int> &assignment) {
    for (int colors = 1; colors <= input.settings.registers; colors++) {
        if (dsaturColorWithN(input.webs, input.interferenceGraph, colors, excludedWebs, assignment)) {
            registersUsed = colors;
            return true;
        }
    }
    return false;
}

static AllocationResult runFreeAllocation(const ParsedInput &input) {
    AllocationResult result = makeBaseResult(input);
    map<int, int> assignment;
    int registersUsed = 0;

    if (tryDsaturUpToLimit(input, set<int>(), registersUsed, assignment)) {
        result.success = true;
        result.registersUsed = registersUsed;
        result.webToRegister = assignment;
        result.warning = "free algorithm used DSATUR backtracking";
        return result;
    }

    vector<int> candidates = spillPriority(input.webs, result.interference);
    for (int spillCount = 1; spillCount <= static_cast<int>(candidates.size()); spillCount++) {
        vector<int> indices(spillCount);
        for (int i = 0; i < spillCount; i++) indices[i] = i;

        do {
            set<int> excludedWebs;
            for (int index : indices) {
                excludedWebs.insert(candidates[index]);
            }

            if (tryDsaturUpToLimit(input, excludedWebs, registersUsed, assignment)) {
                result.success = true;
                result.registersUsed = registersUsed;
                result.webToRegister = assignment;
                result.spilledWebs = excludedWebs;
                result.warning = "free algorithm used DSATUR backtracking with minimum-count heuristic spilling";
                return result;
            }
        } while (nextCombination(indices, static_cast<int>(candidates.size())));
    }

    result.registersUsed = 0;
    for (const Web &web : result.webs) {
        result.spilledWebs.insert(web.id);
    }
    result.warning = "free allocation could not find a valid assignment";
    return result;
}

AllocationResult runAllocation(const string &rangesPath, const string &registersPath) {
    ParsedInput input = loadInputData(rangesPath, registersPath);

    if (input.settings.algorithm == "basic") {
        return runBasicAllocation(input);
    }

    if (input.settings.algorithm == "spilling") {
        return runSpillingAllocation(input);
    }

    if (input.settings.algorithm == "splitting") {
        return runSplittingAllocation(input);
    }

    if (input.settings.algorithm == "free") {
        return runFreeAllocation(input);
    }

    throw runtime_error("algorithm '" + input.settings.algorithm + "' is parsed but not implemented yet");
}
