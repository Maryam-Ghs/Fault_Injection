#include <iostream>
#include <vector>
#include <functional>

/* LLM input variant 10: large-safe-stress */

int main() {
    // Problem definition: y' = -2 * t * y, exact solution y = exp(-t^2)
    // (Only float arithmetic is used)
    std::function<float(float,float)> f = [](float t, float y) -> float {
        return -2.f * t * y;
    };

    // Parameters (larger but safe arrays)
    int nsteps = 2000;          // total number of steps
    float hstep = 0.01f;        // step size

    // Memory containers (std::vector)
    std::vector<float> ts(nsteps + 1);
    std::vector<float> ys(nsteps + 1);
    std::vector<float> fs(nsteps + 1);

    // Initial condition
    ts[0] = 0.f;
    ys[0] = 1.f;                 // y(0) = 1
    fs[0] = f(ts[0], ys[0]);

    // ---- Warm‑up phase: 3 Euler steps to seed 4‑step Adams–Bashforth ----
    for (int i = 0; i < 3; ++i) {
        ts[i + 1] = ts[i] + hstep;
        ys[i + 1] = ys[i] + hstep * fs[i];
        fs[i + 1] = f(ts[i + 1], ys[i + 1]);
    }

    // ---- Main integration: 4‑step Adams–Bashforth (branch‑minimized) ----
    int i = 3;
    while (i < nsteps) {
        // Reordered arithmetic: combine terms before scaling
        float term = (55.f * fs[i] - 59.f * fs[i - 1]) + (37.f * fs[i - 2] - 9.f * fs[i - 3]);
        ts[i + 1] = ts[i] + hstep;
        ys[i + 1] = ys[i] + hstep * term / 24.f;
        fs[i + 1] = f(ts[i + 1], ys[i + 1]);
        ++i;
    }

    // ---- Output results ----
    for (int k = 0; k <= nsteps; ++k) {
        std::cout << "t = " << ts[k] << "   y ≈ " << ys[k] << '\n';
    }
    return 0;
}
