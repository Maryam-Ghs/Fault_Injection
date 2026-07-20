#include <iostream>
#include <vector>

int main()
{
    /* LLM input variant 9: medium-deterministic-random */
    // deterministic test vectors (number of lines)
    std::vector<int> qty = {3, 14, 7, 2, 19, 11, 5, 8, 13, 6, 17, 4, 9, 12, 16};

    // manual loop unrolling: handle two elements per iteration
    int idx = 0;
    int total = qty.size();

    while (idx + 1 < total)
    {
        int n0 = qty[idx];
        int inc0 = n0 + 1;
        int mul0 = n0 * inc0;
        int half0 = mul0 / 2;
        int regions0 = half0 + 1;
        std::cout << "Lines: " << n0 << " => Regions: " << regions0 << '\n';

        int n1 = qty[idx + 1];
        int inc1 = n1 + 1;
        int mul1 = n1 * inc1;
        int half1 = mul1 / 2;
        int regions1 = half1 + 1;
        std::cout << "Lines: " << n1 << " => Regions: " << regions1 << '\n';

        idx += 2;
    }

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
