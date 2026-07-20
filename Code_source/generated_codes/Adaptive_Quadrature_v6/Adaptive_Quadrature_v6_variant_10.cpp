#include <iostream>
#include <vector>
#include <cmath>   // for fabs, sin, cos, exp, etc.

/* LLM input variant 10: large-safe-stress */

int main()
{
    // -----------------------------------------------------------------
    // 1.  Verbose description of the adaptive quadrature (Simpson) method
    // -----------------------------------------------------------------
    // The algorithm recursively (here iteratively with a stack) refines
    // an interval [left, right] until the estimated Simpson error is
    // below a user‑provided tolerance.  All calculations are performed
    // with the single‑precision type `float` as required.
    // -----------------------------------------------------------------

    // -----------------------------------------------------------------
    // 2.  Define the integrand as a lambda (no external function needed).
    //     The lambda captures nothing and uses only float arithmetic.
    // -----------------------------------------------------------------
    auto integrand = [](float x) -> float
    {
        // Example: a function with a steep peak near zero and a long tail.
        // f(x) = sin(x) / (1 + x*x)
        return sinf(x) / (1.0f + x * x);
    };

    // -----------------------------------------------------------------
    // 3.  Prepare a list of test intervals that stress edge cases.
    // -----------------------------------------------------------------
    std::vector<std::pair<float, float>> interval_list;
    interval_list.push_back(std::make_pair(0.0f, 0.0f));                         // zero‑length
    interval_list.push_back(std::make_pair(0.0f, 1e-8f));                        // extremely small
    interval_list.push_back(std::make_pair(-1e6f, 1e6f));                        // very large symmetric
    interval_list.push_back(std::make_pair(-1e3f, -500.0f));                      // large negative range
    interval_list.push_back(std::make_pair(500.0f, 1000.0f));                    // large positive range
    interval_list.push_back(std::make_pair(-0.001f, 0.001f));                    // tiny around zero
    interval_list.push_back(std::make_pair(3.14159f, 6.28318f));                 // one full period of sin
    interval_list.push_back(std::make_pair(-10.0f, 10.0f));                      // moderate symmetric
    interval_list.push_back(std::make_pair(1e2f, 1e2f + 1e-5f));                 // small interval at large magnitude
    interval_list.push_back(std::make_pair(-1e5f, -1e5f + 1e-4f));               // tiny interval far negative

    // -----------------------------------------------------------------
    // 4.  Parameters for the adaptive process (all floats, no const)
    // -----------------------------------------------------------------
    float tolerance = 1e-4f;          // Desired absolute error per interval
    int    max_iter  = 100000;        // Safeguard against infinite loops

    // -----------------------------------------------------------------
    // 5.  Helper structure stored inside a vector (the explicit stack)
    // -----------------------------------------------------------------
    struct Chunk
    {
        float left;      // left endpoint of the sub‑interval
        float right;     // right endpoint of the sub‑interval
        float f_left;    // f(left)
        float f_right;   // f(right)
        float f_mid;     // f((left+right)/2)
        float simpson;   // Simpson estimate on this interval
        float eps;       // local tolerance for this chunk
    };

    // -----------------------------------------------------------------
    // 6.  Process each interval independently and print the result.
    // -----------------------------------------------------------------
    for (size_t idx = 0; idx < interval_list.size(); ++idx)
    {
        float a = interval_list[idx].first;
        float b = interval_list[idx].second;

        // -----------------------------------------------------------------
        // 6.1  Quick check for degenerate interval (zero length)
        // -----------------------------------------------------------------
        if (fabsf(b - a) < 1e-12f)
        {
            std::cout << "Interval " << idx + 1 << " [" << a << ", " << b
                      << "] -> Integral ≈ 0 (degenerate interval)" << std::endl;
            continue;
        }

        // -----------------------------------------------------------------
        // 6.2  Initialise the stack with the whole interval.
        // -----------------------------------------------------------------
        std::vector<Chunk> stack;
        Chunk root;
        root.left    = a;
        root.right   = b;
        // Reordered arithmetic: compute midpoint as a/2 + b/2 instead of (a+b)/2
        root.f_mid   = integrand(a * 0.5f + b * 0.5f);
        root.f_left  = integrand(a);
        root.f_right = integrand(b);
        // Simpson rule: (h/6)*(f_left + 4*f_mid + f_right), with h = b - a
        root.simpson = ((b - a) / 6.0f) *
                       (root.f_left + (root.f_mid + root.f_mid) + root.f_right);
        root.eps     = tolerance;
        stack.push_back(root);

        // -----------------------------------------------------------------
        // 6.3  Adaptive loop – pop a chunk, check error, possibly split.
        // -----------------------------------------------------------------
        float total = 0.0f;            // Accumulator for this interval
        int   counter = 0;             // Iteration counter

        while (!stack.empty())
        {
            // Pop the last element (LIFO) – manual pop to avoid extra functions
            Chunk current = stack.back();
            stack.pop_back();

            // Midpoints of the left and right halves (reordered arithmetic)
            float mid_left  = current.left * 0.5f + current.f_mid * 0.5f; // not used directly
            float mid_right = current.f_mid * 0.5f + current.right * 0.5f; // not used directly

            // Compute new mid‑points for sub‑intervals
            float left_mid   = (current.left + (current.left + current.right) * 0.5f) * 0.5f;
            float right_mid  = ((current.left + current.right) * 0.5f + current.right) * 0.5f;

            // Function evaluations at the new points
            float f_left_mid  = integrand(left_mid);
            float f_right_mid = integrand(right_mid);

            // Simpson estimates on the left and right halves
            float sim_left  = ((current.f_left + (f_left_mid + f_left_mid) + current.f_mid) *
                              ((current.f_mid - current.left) / 6.0f));
            float sim_right = ((current.f_mid + (f_right_mid + f_right_mid) + current.f_right) *
                              ((current.right - current.f_mid) / 6.0f));

            // Estimate of the error (difference of combined Simpson vs. parent)
            float err = fabsf(sim_left + sim_right - current.simpson);

            // -----------------------------------------------------------------
            // 6.4  Decision: accept or subdivide
            // -----------------------------------------------------------------
            if (err <= 15.0f * current.eps || counter > max_iter)
            {
                // Accept this chunk – add corrected Simpson value
                total += sim_left + sim_right + err / 15.0f;
            }
            else
            {
                // Subdivide: push right half first, then left half (stack order)
                Chunk right_chunk;
                right_chunk.left    = (current.left + current.right) * 0.5f;
                right_chunk.right   = current.right;
                right_chunk.f_left  = current.f_mid;
                right_chunk.f_right = current.f_right;
                right_chunk.f_mid   = f_right_mid;
                right_chunk.simpson = ((right_chunk.right - right_chunk.left) / 6.0f) *
                                      (right_chunk.f_left + (right_chunk.f_mid + right_chunk.f_mid) + right_chunk.f_right);
                right_chunk.eps     = current.eps * 0.5f;
                stack.push_back(right_chunk);

                Chunk left_chunk;
                left_chunk.left    = current.left;
                left_chunk.right   = (current.left + current.right) * 0.5f;
                left_chunk.f_left  = current.f_left;
                left_chunk.f_right = current.f_mid;
                left_chunk.f_mid   = f_left_mid;
                left_chunk.simpson = ((left_chunk.right - left_chunk.left) / 6.0f) *
                                     (left_chunk.f_left + (left_chunk.f_mid + left_chunk.f_mid) + left_chunk.f_right);
                left_chunk.eps     = current.eps * 0.5f;
                stack.push_back(left_chunk);
            }

            ++counter;
        }

        // -----------------------------------------------------------------
        // 7.  Output the final result for this interval.
        // -----------------------------------------------------------------
        std::cout << "Interval " << idx + 1 << " [" << a << ", " << b
                  << "] -> Integral ≈ " << total
                  << " (iterations: " << counter << ")" << std::endl;
    }

    // -----------------------------------------------------------------
    // 8.  End of program – all work performed inside main().
    // -----------------------------------------------------------------
    return 0;
}
