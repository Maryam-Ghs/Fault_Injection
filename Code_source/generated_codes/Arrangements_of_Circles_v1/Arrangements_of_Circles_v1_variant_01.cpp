#include <iostream>
#include <vector>

// LLM input variant 1: minimal-boundary
//-----------------------------------------------------------
// Helper: calculate the maximum number of regions created
// by a given count of circles using the formula:
// regions = n^2 - n + 2
//-----------------------------------------------------------
int countRegions(int numCircles)
{
    // step A: square the circle count
    int sq = numCircles * numCircles;

    // step B: subtract the original count
    int diff = sq - numCircles;

    // step C: add the constant term
    int total = diff + 2;

    return total;
}

//-----------------------------------------------------------
// Helper: display results for a list of circle counts
//-----------------------------------------------------------
void showResults(const std::vector<int>& data)
{
    // Use a while‑loop to demonstrate a different iteration style
    size_t idx = 0;
    while (idx < data.size())
    {
        int circles = data[idx];
        int regions = countRegions(circles);

        std::cout << "With " << circles << " circles -> "
                  << regions << " regions\n";

        ++idx;               // move to the next element
    }
}

//-----------------------------------------------------------
// Entry point
//-----------------------------------------------------------
int main()
{
    // Small predefined array of circle quantities (minimal boundary)
    std::vector<int> sample = {0, 1};

    // Compute and print the arrangement counts
    showResults(sample);

    return 0;
}
