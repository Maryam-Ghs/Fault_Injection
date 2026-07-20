/* LLM input variant 6: ordered-structured */
// version #7
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
    // Stack‑allocated array of test cases (ordered and structured)
    struct TestCase { float a; float b; int n; };
    TestCase cases[5];

    // 1. Small negative interval
    cases[0].a = -500.0f; cases[0].b = -400.0f; cases[0].n = 100;
    // 2. Next negative interval, shifted upward
    cases[1].a = -300.0f; cases[1].b = -200.0f; cases[1].n = 200;
    // 3. Positive interval crossing zero
    cases[2].a = 0.0f;    cases[2].b = 100.0f;  cases[2].n = 300;
    // 4. Larger positive interval
    cases[3].a = 200.0f;  cases[3].b = 300.0f;  cases[3].n = 400;
    // 5. Highest positive interval
    cases[4].a = 400.0f;  cases[4].b = 500.0f;  cases[4].n = 500;

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
