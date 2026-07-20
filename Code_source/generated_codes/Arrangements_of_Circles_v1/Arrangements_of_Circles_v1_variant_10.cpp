#include <iostream>
#include <vector>

// LLM input variant 10: large-safe-stress
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
    // Large but safe stress test: 1 through 1000
    std::vector<int> sample;
    sample.reserve(1000);
    for (int i = 1; i <= 1000; ++i)
    {
        sample.push_back(i);
    }

    showResults(sample);
    return 0;
}
