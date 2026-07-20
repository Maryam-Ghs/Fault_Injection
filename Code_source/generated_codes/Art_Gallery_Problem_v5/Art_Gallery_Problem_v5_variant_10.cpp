/* LLM input variant 10: large-safe-stress */
// Art Gallery Problem – version #5 (large-safe-stress)
// Large deterministic input that stays within memory and time limits.

#include <iostream>
#include <vector>

int main()
{
    // ----- generate deterministic large input ------------------------------------
    int vCount = 1000;   // upper‑bounded safe size

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(vCount);
    ys.reserve(vCount);

    int i = 0;
    while (i < vCount)
    {
        // pattern: (i*i % 1000, (i*123) % 1000) ensures values stay in grid
        xs.push_back( (i * i) % 1000 );
        ys.push_back( (i * 123) % 1000 );
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
