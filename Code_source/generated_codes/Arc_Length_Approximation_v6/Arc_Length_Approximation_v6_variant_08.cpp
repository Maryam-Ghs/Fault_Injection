/* LLM input variant 8: sparse-skewed */
#include <vector>
#include <cstdio>
#include <cmath>

int main() {
    // -------------------------------------------------
    // Version #6 – Arc Length Approximation (loop‑heavy)
    // -------------------------------------------------

    // generate a sparse, highly‑skewed dataset
    int count = 1000;                      // number of points
    std::vector<float> xs(count);
    std::vector<float> ys(count);

    // simple linear‑congruential generator (only for y‑noise)
    int seed = 12345;
    for (int i = 0; i < count; ++i) {
        if (i < 30) {
            // dense cluster near 0
            xs[i] = (float)i / 29.0f * 0.02f;          // range [0,0.02]
        } else if (i < 40) {
            // repeated value creates zero‑length segments
            xs[i] = 0.5f;
        } else {
            // sparse tail near 0.9‑1.0
            xs[i] = 0.9f + (float)(i - 40) / (float)(count - 41) * 0.1f;
        }

        // produce a pseudo‑random offset
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        float rnd = (float)(seed & 0xFFFF) / 65535.0f;   // [0,1]

        // y = sin(2πx) + small random perturbation
        float theta = xs[i] * 6.283185307f;              // 2π
        ys[i] = sinf(theta) + 0.05f * (rnd - 0.5f);
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
