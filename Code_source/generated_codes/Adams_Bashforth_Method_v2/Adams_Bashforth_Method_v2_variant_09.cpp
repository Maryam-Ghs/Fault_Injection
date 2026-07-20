#include <iostream>
#include <vector>
#include <functional>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // Problem definition: y' = -1.5 * t * y + 0.3 * sin(t), exact solution unknown
    // (Only float arithmetic is used)
    std::function<float(float,float)> f = [](float t, float y) -> float {
        return -1.5f * t * y + 0.3f * std::sin(t);
    };

    // Parameters (medium-sized deterministic pseudo‑random input)
    int nsteps = 23;          // total number of steps
    float hstep = 0.07f;      // step size

    // Memory containers (std::vector)
    std::vector<float> ts(nsteps + 1);
    std::vector<float> ys(nsteps + 1);
    std::vector<float> fs(nsteps + 1);

    // Initial condition
    ts[0] = 0.f;
    ys[0] = 0.9f;                 // y(0) = 0.9 (deterministic variant)
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
