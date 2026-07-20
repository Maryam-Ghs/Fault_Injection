#include <iostream>
#include <vector>

int main()
{
    /* LLM input variant 10: large-safe-stress */
    // deterministic test vectors (number of lines)
    std::vector<int> qty = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
        50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
        60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
        70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
        90, 91, 92, 93, 94, 95, 96, 97, 98, 99
    };

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
