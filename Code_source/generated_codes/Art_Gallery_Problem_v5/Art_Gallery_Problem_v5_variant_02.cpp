/* LLM input variant 2: small-diverse */
// Art Gallery Problem – version #5 (small-diverse)
// Generates a small but varied deterministic polygon.

#include <iostream>
#include <vector>

int main()
{
    // ----- generate deterministic input ----------------------------------------
    int vCount = 10;   // small diverse set

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(vCount);
    ys.reserve(vCount);

    int i = 0;
    while (i < vCount)
    {
        // predetermined diverse coordinates
        switch (i)
        {
            case 0: xs.push_back(10); ys.push_back(20); break;
            case 1: xs.push_back(200); ys.push_back(150); break;
            case 2: xs.push_back(400); ys.push_back(300); break;
            case 3: xs.push_back(600); ys.push_back(450); break;
            case 4: xs.push_back(800); ys.push_back(600); break;
            case 5: xs.push_back(123); ys.push_back(456); break;
            case 6: xs.push_back(321); ys.push_back(654); break;
            case 7: xs.push_back(50); ys.push_back(75); break;
            case 8: xs.push_back(700); ys.push_back(250); break;
            case 9: xs.push_back(999); ys.push_back(999); break;
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
