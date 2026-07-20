#include <iostream>
#include <vector>
#include <cmath>   // for fabs, sin, cos, exp, etc.

/* LLM input variant 6: ordered-structured */

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
    // 3.  Prepare a highly structured list of test intervals (sorted and symmetric).
    // -----------------------------------------------------------------
    std::vector<std::pair<float, float>> interval_list;
    const int num_intervals = 20;               // total intervals, even number for symmetry
    const float start = -10.0f;
    const float step = 1.0f;                   // uniform step size
    for (int i = 0; i < num_intervals; ++i)
    {
        float left = start + i * step;
        float right = left + step;
        interval_list.emplace_back(left, right);
    }

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
