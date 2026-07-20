/* LLM input variant 8: sparse-skewed */
// Art Gallery Problem – version #5 (sparse-skewed)
// Mostly empty space with a few clustered points.

#include <iostream>
#include <vector>

int main()
{
    // ----- generate deterministic input ----------------------------------------
    int vCount = 20;   // enough to show sparsity

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(vCount);
    ys.reserve(vCount);

    int i = 0;
    while (i < vCount)
    {
        if (i < 3)
        {
            // a tight cluster at the origin
            xs.push_back(i);
            ys.push_back(i);
        }
        else if (i < 6)
        {
            // another cluster far away
            xs.push_back(900 + i);
            ys.push_back(900 + i);
        }
        else
        {
            // sparse points spread across the grid
            xs.push_back(i * 40);
            ys.push_back(1000 - i * 30);
        }
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
