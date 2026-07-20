#include <vector>
#include <cstdio>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // -------------------------------------------------
    // Version #6 – Arc Length Approximation (loop‑heavy)
    // -------------------------------------------------

    // generate a medium‑sized deterministic‑looking dataset
    int count = 1100;                      // number of points (variant)
    std::vector<float> xs(count);
    std::vector<float> ys(count);

    // simple linear‑congruential generator (for y‑noise)
    int seed = 987654321;
    for (int i = 0; i < count; ++i) {
        // quadratic spacing of x in [0,1]
        float t = static_cast<float>(i) / static_cast<float>(count - 1);
        xs[i] = t * t;

        // produce a pseudo‑random offset
        seed = (seed * 1664525 + 1013904223) & 0x7fffffff;
        float rnd = (static_cast<float>(seed & 0xFFFF) / 65535.0f) - 0.5f;   // [-0.5,0.5]

        // y = sin(2πx) + 0.07 * random perturbation
        float theta = xs[i] * 6.283185307f;              // 2π
        ys[i] = sinf(theta) + 0.07f * rnd;
    }

    // -------------------------------------------------
    // Piecewise‑linear arc length computation
    // -------------------------------------------------
    float length = 0.0f;
    for (int k = 1; k < count; ++k) {
        float dx = xs[k] - xs[k - 1];
        float dy = ys[k] - ys[k - 1];

        // reorder: square first, then sum
        float sqdx = dx * dx;
        float sqdy = dy * dy;

        length += sqrtf(sqdx + sqdy);
    }

    // -------------------------------------------------
    // Output
    // -------------------------------------------------
    printf("Arc length approximation (Version 6): %f\n", length);
    return 0;
}
