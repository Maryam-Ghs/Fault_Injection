/* LLM input variant 7: reverse-adversarial */
// Art Gallery Problem – version #5 (reverse-adversarial)
// Vertices are in reverse order, representing a worst‑case ordering.

#include <iostream>
#include <vector>

int main()
{
    // ----- generate deterministic input ----------------------------------------
    int vCount = 12;   // moderate size for adversarial ordering

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(vCount);
    ys.reserve(vCount);

    // generate ascending then store in reverse
    std::vector<int> tempX;
    std::vector<int> tempY;
    tempX.reserve(vCount);
    tempY.reserve(vCount);

    int i = 0;
    while (i < vCount)
    {
        tempX.push_back(i * 50);
        tempY.push_back((vCount - i) * 30);
        ++i;
    }

    // reverse the order into xs/ys
    for (int k = vCount - 1; k >= 0; --k)
    {
        xs.push_back(tempX[k]);
        ys.push_back(tempY[k]);
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
