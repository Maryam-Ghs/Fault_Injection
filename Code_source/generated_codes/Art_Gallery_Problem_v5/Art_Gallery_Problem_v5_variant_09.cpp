/* LLM input variant 9: medium-deterministic-random */
// Art Gallery Problem – version #5 (medium-deterministic-random)
// Medium‑sized pseudo‑random but deterministic data.

#include <iostream>
#include <vector>

int main()
{
    // ----- generate deterministic pseudo‑random input ----------------------------
    int vCount = 50;   // medium size

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(vCount);
    ys.reserve(vCount);

    // simple linear congruential generator (LCG) with fixed seed
    unsigned int seed = 12345;
    const unsigned int a = 1103515245;
    const unsigned int c = 12345;
    const unsigned int m = 1u << 31;

    int i = 0;
    while (i < vCount)
    {
        seed = (a * seed + c) % m;
        int x = static_cast<int>(seed % 1000);
        seed = (a * seed + c) % m;
        int y = static_cast<int>(seed % 1000);
        xs.push_back(x);
        ys.push_back(y);
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
