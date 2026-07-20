#include <iostream>
#include <vector>

// LLM input variant 5: duplicate-heavy
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
    // Repeated duplicate values
    std::vector<int> sample = {2, 2, 2, 3, 3, 4, 4, 4, 4};

    showResults(sample);
    return 0;
}
