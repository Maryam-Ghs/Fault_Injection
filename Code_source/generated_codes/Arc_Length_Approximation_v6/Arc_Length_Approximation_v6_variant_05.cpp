#include <vector>
#include <cstdio>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // -------------------------------------------------
    // Version #6 – Arc Length Approximation (loop‑heavy)
    // -------------------------------------------------

    // generate a dataset with many duplicate points
    int count = 1000;                      // number of points
    std::vector<float> xs(count);
    std::vector<float> ys(count);

    // simple linear‑congruential generator (only for y‑noise)
    int seed = 12345;
    int i = 0;
    int unique_vals = (count + 1) / 2;     // each x value will appear twice
    float last_rnd = 0.0f;

    while (i < count) {
        // duplicate x values: each unique x repeats twice
        int idx = i / 2;
        xs[i] = (float)idx / (float)(unique_vals - 1);

        // produce a pseudo‑random offset, but reuse for the second duplicate
        if (i % 2 == 0) {
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            last_rnd = (float)(seed & 0xFFFF) / 65535.0f;   // [0,1]
        }
        float rnd = last_rnd;

        // y = sin(2πx) + small random perturbation (identical for duplicates)
        float theta = xs[i] * 6.283185307f;              // 2π
        ys[i] = sinf(theta) + 0.05f * (rnd - 0.5f);

        ++i;
    }

    // -------------------------------------------------
    // Piecewise‑linear arc length computation
    // -------------------------------------------------
    float length = 0.0f;
    int k = 1;
    while (k < count) {
        float dx = xs[k] - xs[k - 1];
        float dy = ys[k] - ys[k - 1];

        // reorder: square first, then sum
        float sqdx = dx * dx;
        float sqdy = dy * dy;

        length += sqrtf(sqdx + sqdy);
        ++k;
    }

    // -------------------------------------------------
    // Output
    // -------------------------------------------------
    printf("Arc length approximation (Version 6): %f\n", length);
    return 0;
}
