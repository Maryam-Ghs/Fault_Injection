/* LLM input variant 6: ordered-structured */
#include <vector>
#include <cstdio>
#include <cmath>

int main() {
    // -------------------------------------------------
    // Version #6 – Arc Length Approximation (loop‑heavy)
    // -------------------------------------------------

    // generate a medium‑sized ordered‑structured dataset
    int count = 2000;                      // number of points
    std::vector<float> xs(count);
    std::vector<float> ys(count);

    int i = 0;
    while (i < count) {
        // linearly spaced x in [-1, 1]
        xs[i] = -1.0f + 2.0f * static_cast<float>(i) / static_cast<float>(count - 1);

        // y = x^2 (perfectly symmetric, smooth curve)
        ys[i] = xs[i] * xs[i];

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
