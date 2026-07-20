/* LLM input variant 7: reverse-adversarial */
// Arrangements of Circles – version #7
// Implements a simple combinatorial / geometric demo using only int and float.
// No external input, everything is generated inside main().

#include <iostream>
#include <vector>

int main() {
    // -----------------------------------------------------------------
    // 1) Prepare test data – each sub‑vector holds radii of circles.
    // -----------------------------------------------------------------
    std::vector< std::vector<float> > pack = {
        {9.0f, 8.5f, 7.5f, 6.0f, 5.0f},   // descending large values
        {4.0f, 3.0f, 2.0f, 1.0f},        // simple reverse order
        {10.0f},                         // single element (worst case for factorial loop)
        {12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f} // longer reverse sequence
    };

    // -----------------------------------------------------------------
    // 2) Loop over all test cases.
    // -----------------------------------------------------------------
    for (int idx = 0; idx < (int)pack.size(); ++idx) {
        // Grab the current radii list.
        std::vector<float> &rlist = pack[idx];

        // -----------------------------------------------------------------
        // 3) Compute total area using a fused expression (π ≈ 3.14f).
        // -----------------------------------------------------------------
        float pi = 3.14f;
        float totalArea = 0.0f;
        for (int i = 0; i < (int)rlist.size(); ++i) {
            // area contribution = π * r * r
            totalArea = totalArea + pi * rlist[i] * rlist[i];
        }

        // -----------------------------------------------------------------
        // 4) Compute number of distinct linear arrangements = n!
        //    (factorial built with a nested loop to keep it loop‑heavy).
        // -----------------------------------------------------------------
        int circles = (int)rlist.size();
        int arrangementCount = 1;
        for (int a = 1; a <= circles; ++a) {
            // multiply current factorial by a using an inner accumulation loop
            int product = 0;
            for (int b = 0; b < a; ++b) {
                product = product + arrangementCount;
            }
            arrangementCount = product;
        }

        // -----------------------------------------------------------------
        // 5) Output the results.
        // -----------------------------------------------------------------
        std::cout << "Case " << idx << " : ";
        std::cout << circles << " circle(s)  |  ";
        std::cout << "Total area = " << totalArea << "  |  ";
        std::cout << "Arrangements = " << arrangementCount << std::endl;
    }

    return 0;
}
