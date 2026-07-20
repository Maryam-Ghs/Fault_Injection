#include <iostream>
#include <vector>

// LLM input variant 6: ordered-structured
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
    for (int i = 1; i <= 5; ++i)
        data.push_back(i);
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
