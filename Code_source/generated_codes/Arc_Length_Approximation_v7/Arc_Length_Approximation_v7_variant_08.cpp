// version #7
/* LLM input variant 8: sparse-skewed */
#include <cstdio>
#include <cmath>

float f_of_x(float x) {
    // Example function: f(x) = x^2
    return x * x;
}

float seg_len(float x0, float x1) {
    // Expanded multi‑step computation of segment length
    float dx = x1 - x0;
    float y0 = f_of_x(x0);
    float y1 = f_of_x(x1);
    float dy = y1 - y0;

    float dx_sq = dx * dx;
    float dy_sq = dy * dy;
    float sum_sq = dx_sq + dy_sq;
    float length = sqrtf(sum_sq);
    return length;
}

float approx_arc_len(float a, float b, int n) {
    // Guard against degenerate cases
    if (a == b) return 0.0f;
    // Ensure forward direction; arc length is always positive
    float lo = a;
    float hi = b;
    if (b < a) {
        lo = b;
        hi = a;
    }

    // Step size (float arithmetic only)
    float step = (hi - lo) / (float)n;

    // Manual loop unrolling: two segments per iteration
    float total = 0.0f;
    int idx = 0;
    float x_prev = lo;

    for (; idx <= n - 2; idx += 2) {
        // first segment
        float x_mid = x_prev + step;
        total += seg_len(x_prev, x_mid);

        // second segment
        float x_next = x_mid + step;
        total += seg_len(x_mid, x_next);

        // advance
        x_prev = x_next;
    }

    // leftover segment if n is odd
    if (idx < n) {
        float x_last = x_prev + step;
        total += seg_len(x_prev, x_last);
    }

    return total;
}

int main() {
    // Stack‑allocated array of test cases (sparse‑skewed distribution)
    struct TestCase { float a; float b; int n; };
    TestCase cases[5];

    // 1. Zero‑length interval (baseline)
    cases[0].a = 0.0f;  cases[0].b = 0.0f;  cases[0].n = 10;
    // 2. Tiny interval, single subdivision
    cases[1].a = 1000.0f;  cases[1].b = 1000.1f;  cases[1].n = 1;
    // 3. Very small interval, massive subdivisions (dense)
    cases[2].a = -0.001f;  cases[2].b = 0.001f;  cases[2].n = 100000;
    // 4. Large symmetric interval, few subdivisions (sparse)
    cases[3].a = -5000.0f; cases[3].b = 5000.0f; cases[3].n = 10;
    // 5. Extremely large interval, moderate subdivisions
    cases[4].a = 0.0f;  cases[4].b = 1000000.0f; cases[4].n = 100;

    for (int i = 0; i < 5; ++i) {
        float a_val = cases[i].a;
        float b_val = cases[i].b;
        int   n_val = cases[i].n;

        float result = approx_arc_len(a_val, b_val, n_val);
        std::printf("Case %d: a=%f b=%f n=%d => arc length ≈ %f\n",
                    i + 1, a_val, b_val, n_val, result);
    }

    return 0;
}
