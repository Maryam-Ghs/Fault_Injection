/ * LLM input variant 10: large-safe-stress * /
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
    // Stack‑allocated array of test cases (expanded for stress)
    struct TestCase { float a; float b; int n; };
    TestCase cases[10];

    // 1. Zero‑length interval
    cases[0].a = 0.0f;   cases[0].b = 0.0f;    cases[0].n = 10;
    // 2. Reverse interval
    cases[1].a = 5.0f;   cases[1].b = -5.0f;   cases[1].n = 20;
    // 3. Small interval, many subdivisions
    cases[2].a = 0.0f;   cases[2].b = 1.0f;    cases[2].n = 1000;
    // 4. Large symmetric interval
    cases[3].a = -100.0f; cases[3].b = 100.0f; cases[3].n = 2000;
    // 5. Very large interval
    cases[4].a = 0.0f;   cases[4].b = 1000.0f; cases[4].n = 5000;
    // 6. Extremely large n on moderate interval
    cases[5].a = -10.0f; cases[5].b = 10.0f;  cases[5].n = 200000;
    // 7. Large interval with moderate n
    cases[6].a = -5000.0f; cases[6].b = 5000.0f; cases[6].n = 10000;
    // 8. Reverse large interval with high n
    cases[7].a = 10000.0f; cases[7].b = -10000.0f; cases[7].n = 50000;
    // 9. Small negative to positive interval, high n
    cases[8].a = -0.5f;   cases[8].b = 0.5f;   cases[8].n = 250000;
    // 10. Very large positive interval, safe n
    cases[9].a = 0.0f;    cases[9].b = 20000.0f; cases[9].n = 100000;

    for (int i = 0; i < 10; ++i) {
        float a_val = cases[i].a;
        float b_val = cases[i].b;
        int   n_val = cases[i].n;

        float result = approx_arc_len(a_val, b_val, n_val);
        std::printf("Case %d: a=%f b=%f n=%d => arc length ≈ %f\n",
                    i + 1, a_val, b_val, n_val, result);
    }

    return 0;
}
