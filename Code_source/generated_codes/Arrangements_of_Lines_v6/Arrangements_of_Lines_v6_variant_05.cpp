#include <iostream>
#include <vector>

// LLM input variant 5: duplicate-heavy
// ------------------------------------------------------------------
// Helper: compute the maximal number of regions created by 'numLines'
//          assuming a general position (no parallels, no three concurrent)
// ------------------------------------------------------------------
int calculateRegions(int numLines)
{
    int plusOne = numLines + 1;
    int product = numLines * plusOne;
    int half = product / 2;
    int totalRegions = half + 1;
    return totalRegions;
}

// ------------------------------------------------------------------
// Helper: produce a small predefined collection of line counts
// ------------------------------------------------------------------
std::vector<int> produceTestData()
{
    std::vector<int> data;
    data.push_back(2);
    data.push_back(2);
    data.push_back(3);
    data.push_back(3);
    data.push_back(3);
    data.push_back(4);
    data.push_back(4);
    return data;
}

// ------------------------------------------------------------------
// Entry point
// ------------------------------------------------------------------
int main()
{
    std::vector<int> lineSamples = produceTestData();
    std::size_t idx = 0;
    while (idx < lineSamples.size())
    {
        int currentLines = lineSamples[idx];
        int computedRegions = calculateRegions(currentLines);
        std::cout << "Lines: " << currentLines
                  << " => Regions: " << computedRegions << '\n';
        ++idx;
    }
    return 0;
}
