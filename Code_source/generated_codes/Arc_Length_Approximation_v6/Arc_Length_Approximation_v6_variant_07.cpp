#include <vector>
#include <cstdio>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

int main() {
    // -------------------------------------------------
    // Version #6 – Arc Length Approximation (loop‑heavy)
    // -------------------------------------------------

    // generate a reverse‑ordered, worst‑case‑like dataset
    int count = 2000;                      // number of points (larger for stress)
    std::vector<float> xs(count);
    std::vector<float> ys(count);

    // xs decreasing from 1 to 0, ys alternating to maximize segment differences
    int i = 0;
    while (i < count) {
        // uniform x in [0,1] but reverse order
        xs[i] = 1.0f - (float)i / (float)(count - 1);

        // y = sin(2πx) with alternating large perturbation
        float theta = xs[i] * 6.283185307f;              // 2π
        float base = sinf(theta);
        if (i % 2 == 0) {
            ys[i] = base + 0.5f;   // upward shift
        } else {
            ys[i] = base - 0.5f;   // downward shift
        }

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
