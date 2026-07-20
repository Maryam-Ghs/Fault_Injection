#include <iostream>
#include <vector>

 /* LLM input variant 5: duplicate-heavy */

int main() {
    // ---- parameters (edge‑case heavy) ----
    float tol = 1e-3f;               // error tolerance
    float minLen = 1e-5f;            // minimal interval length
    int maxIter = 100000;            // safety guard

    // ---- function to integrate (float only) ----
    auto f = [](float x) -> float {
        // rational function with asymptotic behaviour at large |x|
        return 1.0f / (1.0f + x * x);
    };

    // ---- interval representation ----
    struct Part { float l; float r; float val; };

    // ---- generate duplicate‑heavy initial data ----
    std::vector<Part> pending;
    // repeated normal ranges
    for (int i = 0; i < 3; ++i) {
        pending.push_back({-10.0f, -5.0f, 0.0f});
        pending.push_back({-5.0f, 0.0f, 0.0f});
        pending.push_back({0.0f, 5.0f, 0.0f});
        pending.push_back({5.0f, 10.0f, 0.0f});
    }
    // duplicate zero‑length intervals
    for (int i = 0; i < 5; ++i) {
        pending.push_back({3.14159f, 3.14159f, 0.0f});
    }
    // duplicate extremely large intervals
    for (int i = 0; i < 4; ++i) {
        pending.push_back({-1e6f, 1e6f, 0.0f});
    }
    // duplicate negative‑oriented intervals (will be reordered)
    for (int i = 0; i < 3; ++i) {
        pending.push_back({20.0f, 15.0f, 0.0f});
    }
    // duplicate very small intervals
    for (int i = 0; i < 6; ++i) {
        pending.push_back({0.00001f, 0.00002f, 0.0f});
    }

    // ---- containers for finished pieces ----
    std::vector<Part> result;

    // ---- adaptive refinement loop (branch‑minimized) ----
    int iter = 0;
    while (!pending.empty() && iter < maxIter) {
        ++iter;
        Part cur = pending.back();
        pending.pop_back();

        // reorder if needed (branch‑minimized via ternary)
        float a = cur.l, b = cur.r;
        cur.l = (a < b) ? a : b;
        cur.r = (a < b) ? b : a;

        float mid = (cur.l + cur.r) * 0.5f;
        float fl = f(cur.l);
        float fm = f(mid);
        float fr = f(cur.r);

        float coarse = (cur.r - cur.l) * (fl + fr) * 0.5f;                 // trapezoid
        float fine   = (cur.r - cur.l) * (fl + 4.0f * fm + fr) / 6.0f;    // Simpson
        float err    = fine - coarse;
        float segLen = cur.r - cur.l;

        // decide to split or keep (single ternary expression)
        ( (err > tol || err < -tol) && segLen > minLen )
            ? ( pending.push_back({cur.l, mid, 0.0f}),
                pending.push_back({mid, cur.r, 0.0f}) )
            : result.push_back({cur.l, cur.r, fine});
    }

    // ---- compute total integral and output ----
    float total = 0.0f;
    for (int i = 0; i < (int)result.size(); ++i) total += result[i].val;

    std::cout << "Refined intervals : " << result.size() << '\n';
    std::cout << "Estimated integral: " << total << '\n';
    std::cout << "Iterations performed: " << iter << '\n';
    return 0;
}
