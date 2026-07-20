#include <iostream>
#include <vector>

// LLM input variant 7: reverse-adversarial
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
    // Reverse-ordered series to stress order handling
    std::vector<int> sample = {20,19,18,17,16,15,14,13,12,11,
                               10,9,8,7,6,5,4,3,2,1};

    showResults(sample);
    return 0;
}
