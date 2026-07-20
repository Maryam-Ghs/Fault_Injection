/* LLM input variant 6: ordered-structured */
// Art Gallery Problem – version #5 (ordered-structured)
// Vertices are ordered and form a symmetric, structured shape.

#include <iostream>
#include <vector>

int main()
{
    // ----- generate deterministic input ----------------------------------------
    int vCount = 15;   // structured count

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(vCount);
    ys.reserve(vCount);

    int i = 0;
    while (i < vCount)
    {
        // ascending x, y = x * 2, creating a simple monotonic polygon
        xs.push_back(i * 10);
        ys.push_back(i * 20);
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
