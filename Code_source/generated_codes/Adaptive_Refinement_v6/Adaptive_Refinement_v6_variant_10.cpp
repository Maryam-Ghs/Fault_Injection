#include <iostream>
#include <vector>

/* LLM input variant 10: large-safe-stress */

int main() {
    // ---- parameters (edge‑case heavy) ----
    float tol = 5e-4f;                // error tolerance
    float minLen = 1e-6f;             // minimal interval length
    int maxIter = 200000;            // safety guard

    // ---- function to integrate (float only) ----
    auto f = [](float x) -> float {
        // rational function with asymptotic behaviour at large |x|
        return 1.0f / (1.0f + x * x);
    };

    // ---- interval representation ----
    struct Part { float l; float r; float val; };

    // ---- generate edge‑case heavy initial data ----
    std::vector<Part> pending;
    // extremely large intervals
    pending.push_back({-1e8f, -5e7f, 0.0f});
    pending.push_back({-5e7f, 0.0f, 0.0f});
    pending.push_back({0.0f, 5e7f, 0.0f});
    pending.push_back({5e7f, 1e8f, 0.0f});
    // zero‑length interval
    pending.push_back({2.71828f, 2.71828f, 0.0f});
    // very small interval
    pending.push_back({1e-5f, 2e-5f, 0.0f});
    // reversed interval (will be reordered)
    pending.push_back({100.0f, 50.0f, 0.0f});
    // moderate intervals
    pending.push_back({-1000.0f, -500.0f, 0.0f});
    pending.push_back({-500.0f, 0.0f, 0.0f});
    pending.push_back({0.0f, 500.0f, 0.0f});
    pending.push_back({500.0f, 1000.0f, 0.0f});
    // another small interval around zero
    pending.push_back({-0.001f, 0.001f, 0.0f});
    // large symmetric interval
    pending.push_back({-1e5f, 1e5f, 0.0f});

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
