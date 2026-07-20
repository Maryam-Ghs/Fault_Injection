#include <iostream>
#include <vector>

int main()
{
    /* LLM input variant 1: minimal-boundary */
    // deterministic test vectors (number of lines)
    std::vector<int> qty = {0, 1};

    // manual loop unrolling: handle two elements per iteration
    int idx = 0;
    int total = qty.size();

    while (idx + 1 < total)
    {
        // first element
        int n0 = qty[idx];
        int inc0 = n0 + 1;          // expanded step
        int mul0 = n0 * inc0;       // product
        int half0 = mul0 / 2;       // division
        int regions0 = half0 + 1;   // final addition
        std::cout << "Lines: " << n0 << " => Regions: " << regions0 << '\n';

        // second element
        int n1 = qty[idx + 1];
        int inc1 = n1 + 1;
        int mul1 = n1 * inc1;
        int half1 = mul1 / 2;
        int regions1 = half1 + 1;
        std::cout << "Lines: " << n1 << " => Regions: " << regions1 << '\n';

        idx += 2;   // manual step
    }

    // handle possible leftover element
    if (idx < total)
    {
        int n = qty[idx];
        int inc = n + 1;
        int mul = n * inc;
        int half = mul / 2;
        int regions = half + 1;
        std::cout << "Lines: " << n << " => Regions: " << regions << '\n';
    }

    return 0;
}
