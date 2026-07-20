/* LLM input variant 4: signed-extremes */
// Art Gallery Problem – version #5 (signed-extremes)
// Mixes negative, zero, and positive coordinates within safe limits.

#include <iostream>
#include <vector>

int main()
{
    // ----- generate deterministic input ----------------------------------------
    int vCount = 9;   // odd count to exercise floor division

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(vCount);
    ys.reserve(vCount);

    int i = 0;
    while (i < vCount)
    {
        // deterministic signed extremes, staying within [-1000, 1000]
        if (i % 3 == 0)        { xs.push_back(-1000 + i * 10); ys.push_back(-1000 + i * 10); }
        else if (i % 3 == 1)   { xs.push_back(0);               ys.push_back(0); }
        else                    { xs.push_back(1000 - i * 10); ys.push_back(1000 - i * 10); }
        ++i;
    }

    // ----- compute answer --------------------------------------------------------
    int guardsNeeded = xs.size() / 3;

    // ----- output ----------------------------------------------------------------
    std::cout << "Generated polygon with " << vCount << " vertices.\n";
    std::cout << "Minimum number of guards required (theoretical): "
              << guardsNeeded << "\n";

    std::cout << "Sample vertices (first 5):\n";
    int j = 0;
    while (j < 5 && j < vCount)
    {
        std::cout << "  (" << xs[j] << ", " << ys[j] << ")\n";
        ++j;
    }

    return 0;
}
