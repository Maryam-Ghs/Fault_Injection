/* LLM input variant 1: minimal-boundary */
#include <vector>
#include <cstdio>
#include <cmath>

int main() {
    // -------------------------------------------------
    // Version #6 – Arc Length Approximation (loop‑heavy)
    // -------------------------------------------------

    // generate a minimal‑sized dataset (boundary case)
    int count = 2;                      // smallest nontrivial number of points
    std::vector<float> xs(count);
    std::vector<float> ys(count);

    // simple linear‑congruential generator (only for y‑noise)
    int seed = 12345;
    int i = 0;
    while (i < count) {
        // uniform x in [0,1]
        xs[i] = (float)i / (float)(count - 1);

        // produce a pseudo‑random offset
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        float rnd = (float)(seed & 0xFFFF) / 65535.0f;   // [0,1]

        // y = sin(2πx) + small random perturbation
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
