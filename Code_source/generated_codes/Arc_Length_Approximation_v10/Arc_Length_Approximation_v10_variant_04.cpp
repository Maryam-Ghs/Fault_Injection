#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 4: signed-extremes */

int main() {
    // Version identifier
    std::cout << "Arc Length Approximation - Version #10\n";

    // ------------------------------------------------------------
    // 1. Prepare edge‑case heavy input data (all generated internally)
    // ------------------------------------------------------------
    std::vector<float> a_vals;   // start points
    std::vector<float> b_vals;   // end points
    std::vector<float> h_vals;   // step sizes

    // Case 1: zero length interval with signed negative zero start
    a_vals.push_back(-0.0f);
    b_vals.push_back(0.0f);
    h_vals.push_back(0.5f);          // step size irrelevant here

    // Case 2: symmetric interval with zero step (fallback will be used)
    a_vals.push_back(-10.0f);
    b_vals.push_back(10.0f);
    h_vals.push_back(0.0f);          // triggers safe default step

    // Case 3: large interval with moderate step
    a_vals.push_back(-1000000.0f);
    b_vals.push_back(1000000.0f);
    h_vals.push_back(50000.0f);      // reasonable number of segments

    // Case 4: zero length interval where both ends are negative
    a_vals.push_back(-5.0f);
    b_vals.push_back(-5.0f);
    h_vals.push_back(0.2f);          // step size irrelevant here

    // Case 5: small positive interval with a tiny step
    a_vals.push_back(0.0f);
    b_vals.push_back(5.0f);
    h_vals.push_back(0.001f);        // many steps, tests precision

    // ------------------------------------------------------------
    // 2. Process each test case one by one
    // ------------------------------------------------------------
    std::size_t case_idx = 0;
    while (case_idx < a_vals.size()) {
        // --------------------------------------------------------
        // 2.1 Extract raw parameters for this case
        // --------------------------------------------------------
        float a0 = a_vals[case_idx];
        float b0 = b_vals[case_idx];
        float h0 = h_vals[case_idx];

        // --------------------------------------------------------
        // 2.2 Guard against non‑positive step size (edge case safety)
        // --------------------------------------------------------
        if (h0 <= 0.0f) {
            h0 = 0.1f;                 // fallback to a safe default
        }

        // --------------------------------------------------------
        // 2.3 Determine direction of integration (forward or backward)
        // --------------------------------------------------------
        float direction = (b0 >= a0) ? 1.0f : -1.0f;
        float step = direction * h0;   // signed step

        // --------------------------------------------------------
        // 2.4 Compute number of sub‑intervals (as an integer)
        // --------------------------------------------------------
        float interval_len = b0 - a0;
        int   seg_cnt      = static_cast<int>(interval_len / step);
        if (seg_cnt < 0) seg_cnt = -seg_cnt;   // absolute count
        if (seg_cnt == 0) seg_cnt = 1;         // at least one segment

        // --------------------------------------------------------
        // 2.5 Allocate vectors for x and y coordinates
        // --------------------------------------------------------
        std::vector<float> xs;
        std::vector<float> ys;
        xs.reserve(seg_cnt + 1);
        ys.reserve(seg_cnt + 1);

        // --------------------------------------------------------
        // 2.6 Fill x‑coordinates (verbose, step‑by‑step)
        // --------------------------------------------------------
        float cur_x = a0;
        xs.push_back(cur_x);
        int fill_idx = 0;
        while (fill_idx < seg_cnt) {
            cur_x = cur_x + step;       // advance by signed step
            xs.push_back(cur_x);
            ++fill_idx;
        }

        // --------------------------------------------------------
        // 2.7 Compute y‑coordinates using the function f(x) = sin(x)
        // --------------------------------------------------------
        std::size_t y_idx = 0;
        while (y_idx < xs.size()) {
            float argument = xs[y_idx];
            float sin_val  = sinf(argument);   // float version of sin
            ys.push_back(sin_val);
            ++y_idx;
        }

        // --------------------------------------------------------
        // 2.8 Approximate arc length by summing segment lengths
        // --------------------------------------------------------
        float total_len = 0.0f;
        int   seg_idx   = 1;
        while (seg_idx <= seg_cnt) {
            // Differences
            float dx = xs[seg_idx] - xs[seg_idx - 1];
            float dy = ys[seg_idx] - ys[seg_idx - 1];

            // Square terms (expanded multi‑step)
            float dx_sq = dx * dx;
            float dy_sq = dy * dy;

            // Sum of squares (reordered mathematically)
            float sum_sq = dy_sq + dx_sq;

            // Segment length
            float seg_len = sqrtf(sum_sq);

            // Accumulate
            total_len = total_len + seg_len;

            ++seg_idx;
        }

        // --------------------------------------------------------
        // 2.9 Output the result for this case
        // --------------------------------------------------------
        std::cout << "Case " << (case_idx + 1) << ":\n";
        std::cout << "  start = " << a0 << ", end = " << b0 << ", step = " << h0 << "\n";
        std::cout << "  segments used = " << seg_cnt << "\n";
        std::cout << "  approximated arc length = " << total_len << "\n\n";

        // --------------------------------------------------------
        // 2.10 Move to next test case
        // --------------------------------------------------------
        ++case_idx;
    }

    // ------------------------------------------------------------
    // End of program
    // ------------------------------------------------------------
    return 0;
}
