/* LLM input variant 1: minimal-boundary */
// Art Gallery Problem – version #5 (minimal-boundary)
// Generates a deterministic simple polygon with the smallest non‑trivial size
// and prints the theoretical minimum number of guards (⌊n/3⌋).

#include <iostream>
#include <vector>

int main()
{
    // ----- generate deterministic input ----------------------------------------
    // smallest non‑trivial polygon: 3 vertices
    int vCount = 3;

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(vCount);
    ys.reserve(vCount);

    // fill coordinate arrays – using a while‑loop for variety
    int i = 0;
    while (i < vCount)
    {
        // deterministic coordinates forming a simple triangle
        if (i == 0) { xs.push_back(0); ys.push_back(0); }
        else if (i == 1) { xs.push_back(1); ys.push_back(0); }
        else { xs.push_back(0); ys.push_back(1); }
        ++i;
    }

    // ----- compute answer --------------------------------------------------------
    int guardsNeeded = xs.size() / 3;   // xs.size() == vCount

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
