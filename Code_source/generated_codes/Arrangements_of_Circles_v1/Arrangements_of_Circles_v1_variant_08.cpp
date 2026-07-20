#include <iostream>
#include <vector>

// LLM input variant 8: sparse-skewed
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
    // Mostly zeros with a few larger values (sparse, skewed)
    std::vector<int> sample = {0,0,0,0,0,0,0,0,0,0,
                               5,10,15};

    showResults(sample);
    return 0;
}
