#include <vector>
#include <cstdio>
#include <cmath>

/* LLM input variant 3: zeros-and-ones */

int main() {
    // -------------------------------------------------
    // Version #6 – Arc Length Approximation (loop‑heavy)
    // -------------------------------------------------

    // generate a dataset emphasizing zeros and ones
    int count = 1000;                      // number of points
    std::vector<float> xs(count);
    std::vector<float> ys(count);

    int i = 0;
    while (i < count) {
        // alternate between 0 and 1 for both coordinates
        xs[i] = (i % 2 == 0) ? 0.0f : 1.0f;
        ys[i] = (i % 2 == 0) ? 0.0f : 1.0f;
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
