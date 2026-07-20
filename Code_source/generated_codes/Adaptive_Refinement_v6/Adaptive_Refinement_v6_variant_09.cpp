#include <iostream>
#include <vector>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // ---- parameters (medium‑deterministic‑random) ----
    float tol = 5e-4f;                // error tolerance
    float minLen = 5e-6f;             // minimal interval length
    int maxIter = 100000;            // safety guard

    // ---- function to integrate (float only) ----
    auto f = [](float x) -> float {
        // rational function with asymptotic behaviour at large |x|
        return 1.0f / (1.0f + x * x);
    };

    // ---- interval representation ----
    struct Part { float l; float r; float val; };

    // ---- deterministic pseudo‑random generator ----
    const unsigned init_seed = 123456789u;
    unsigned state = init_seed;
    auto lcg = [&state]() -> unsigned {
        state = state * 1664525u + 1013904223u;
        return state;
    };
    auto rand_float = [&](float low, float high) -> float {
        unsigned r = lcg();
        return low + (r / (float)UINT_MAX) * (high - low);
    };

    // ---- generate medium‑deterministic‑random initial data ----
    std::vector<Part> pending;
    const int N = 12;                     // number of random intervals
    for (int i = 0; i < N; ++i) {
        float a = rand_float(-50.0f, 50.0f);
        float b = rand_float(-50.0f, 50.0f);
        pending.push_back({a, b, 0.0f});
    }
    // explicit edge‑case intervals
    pending.push_back({3.1415926f, 3.1415926f, 0.0f});          // zero‑length
    pending.push_back({-2e5f, 2e5f, 0.0f});                    // extremely large
    pending.push_back({25.0f, 20.0f, 0.0f});                    // reversed orientation
    pending.push_back({0.000001f, 0.000005f, 0.0f});          // very small

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
