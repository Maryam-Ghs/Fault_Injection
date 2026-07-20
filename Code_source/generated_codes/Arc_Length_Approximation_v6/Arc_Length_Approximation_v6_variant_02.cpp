#include <vector>
#include <cstdio>
#include <cmath>

/* LLM input variant 2: small-diverse */

int main() {
    // -------------------------------------------------
    // Version #6 – Arc Length Approximation (loop‑heavy)
    // -------------------------------------------------

    // generate a small‑sized diverse dataset
    int count = 7;                      // number of points
    std::vector<float> xs(count);
    std::vector<float> ys(count);

    // simple linear‑congruential generator (only for y‑noise)
    int seed = 98765;
    for (int i = 0; i < count; ++i) {
        // non‑uniform x pattern for variety
        xs[i] = (i == 0) ? 0.0f :
                (i == 1) ? 0.12f :
                (i == 2) ? 0.35f :
                (i == 3) ? 0.55f :
                (i == 4) ? 0.73f :
                (i == 5) ? 0.88f : 1.0f;

        // produce a pseudo‑random offset
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        float rnd = (float)(seed & 0xFFFF) / 65535.0f;   // [0,1]

        // y = sin(2πx) + modest random perturbation
        float theta = xs[i] * 6.283185307f;              // 2π
        ys[i] = sinf(theta) + 0.07f * (rnd - 0.5f);
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
