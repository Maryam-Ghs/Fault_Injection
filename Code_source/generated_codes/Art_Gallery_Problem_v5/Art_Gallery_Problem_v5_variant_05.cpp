/* LLM input variant 5: duplicate-heavy */
// Art Gallery Problem – version #5 (duplicate-heavy)
// Contains many duplicate points and repeated patterns.

#include <iostream>
#include <vector>

int main()
{
    // ----- generate deterministic input ----------------------------------------
    int vCount = 12;   // includes duplicates

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(vCount);
    ys.reserve(vCount);

    int i = 0;
    while (i < vCount)
    {
        // repeat a small set of coordinates three times each
        int idx = i % 4;
        switch (idx)
        {
            case 0: xs.push_back(100); ys.push_back(200); break;
            case 1: xs.push_back(300); ys.push_back(400); break;
            case 2: xs.push_back(500); ys.push_back(600); break;
            case 3: xs.push_back(700); ys.push_back(800); break;
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
