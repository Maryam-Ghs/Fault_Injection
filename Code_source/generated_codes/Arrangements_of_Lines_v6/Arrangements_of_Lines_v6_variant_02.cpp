#include <iostream>
#include <vector>

// LLM input variant 2: small-diverse
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
    data.push_back(1);
    data.push_back(2);
    data.push_back(5);
    data.push_back(10);
    data.push_back(12);
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
