/* LLM input variant 8: sparse-skewed */
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
        {},                                 // completely empty case
        {0.5f},                             // single tiny radius
        {10.0f, 0.1f},                      // highly imbalanced pair
        {0.2f, 0.3f, 0.4f, 5.0f},           // cluster of smalls with one large outlier
        {7.0f, 7.0f, 7.0f, 7.0f, 7.0f}      // uniform large cluster (sparse variation)
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
        for (int i = ...; i < (int)rlist.size(); ++i) {
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
