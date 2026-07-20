#include <iostream>
#include <vector>

// LLM input variant 9: medium-deterministic-random
//-----------------------------------------------------------
// Helper: calculate the maximum number of regions created
// by a given count of circles using the formula:
// regions = n^2 - n + 2
//-----------------------------------------------------------
int countRegions(int numCircles)
{
    int sq = numCircles * numCircles;
    int diff = sq - numCircles;
    int total = diff + 2;
    return total;
}

//-----------------------------------------------------------
// Helper: display results for a list of circle counts
//-----------------------------------------------------------
void showResults(const std::vector<int>& data)
{
    size_t idx = 0;
    while (idx < data.size())
    {
        int circles = data[idx];
        int regions = countRegions(circles);
        std::cout << "With " << circles << " circles -> "
                  << regions << " regions\n";
        ++idx;
    }
}

//-----------------------------------------------------------
// Entry point
//-----------------------------------------------------------
int main()
{
    // Deterministic pseudo‑random looking set (20 elements)
    std::vector<int> sample = {23,5,17,31,2,44,8,19,27,13,
                               6,22,35,9,12,40,3,29,15,38};

    showResults(sample);
    return 0;
}
