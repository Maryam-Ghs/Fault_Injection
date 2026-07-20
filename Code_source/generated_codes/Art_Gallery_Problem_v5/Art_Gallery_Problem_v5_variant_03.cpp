/* LLM input variant 3: zeros-and-ones */
// Art Gallery Problem – version #5 (zeros-and-ones)
// Emphasizes zeros, ones, and minimal values.

#include <iostream>
#include <vector>

int main()
{
    // ----- generate deterministic input ----------------------------------------
    int vCount = 6;   // even number to show zeros and ones

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(vCount);
    ys.reserve(vCount);

    int i = 0;
    while (i < vCount)
    {
        // pattern of 0s and 1s
        xs.push_back(i % 2);                     // 0,1,0,1,...
        ys.push_back((i + 1) % 2);               // 1,0,1,0,...
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
