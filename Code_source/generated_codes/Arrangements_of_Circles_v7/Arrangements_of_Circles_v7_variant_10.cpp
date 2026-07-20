/* LLM input variant 10: large-safe-stress */
// Arrangements of Circles – version #7
// Implements a simple combinatorial / geometric demo using only int and float.
// No external input, everything is generated inside main().

#include <iostream>
#include <vector>

int main() {
    // -----------------------------------------------------------------
    // 1) Prepare test data – each sub‑vector holds radii of circles.
    //    Large but safe sizes (max 12 circles per case to keep factorial in 32‑bit int).
    // -----------------------------------------------------------------
    std::vector< std::vector<float> > pack = {
        {1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 4.5f, 5.0f, 5.5f, 6.0f, 6.5f},
        {0.2f, 0.4f, 0.6f, 0.8f, 1.0f, 1.2f, 1.4f, 1.6f, 1.8f, 2.0f},
        {10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f},
        {2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.1f, 11.2f, 12.3f}
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
