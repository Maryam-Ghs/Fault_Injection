#include <cstdio>
#include <cstdlib>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // problem size and step
    int steps = 1500;
    float stepH = 0.008f;

    // stack arrays for t, y and f(t,y)
    float t[1500];
    float y[1500];
    float f[1500];

    // deterministic pseudo‑random initial condition (linear congruential)
    unsigned int seed = 987654321u;
    seed = (seed * 1664525u + 1013904223u) & 0xFFFFFFFFu;
    y[0] = (float)(seed % 1000) / 100.0f;   // y0 ∈ [0,9.99]
    t[0] = 0.0f;

    // ODE: dy/dt = -y + t   (example)
    f[0] = -y[0] + t[0];

    // first step with explicit Euler (needs no branch)
    t[1] = t[0] + stepH;
    y[1] = y[0] + stepH * f[0];
    f[1] = -y[1] + t[1];

    // Adams–Bashforth predictor + Adams–Moulton corrector (2‑step)
    int i = 2;
    while (i < steps) {
        t[i] = t[i-1] + stepH;                                          // advance time
        // predictor (AB2): ŷ = y_{i-1} + h*(3*f_{i-1} - f_{i-2})/2
        float pred = y[i-1] + stepH * (3.0f*f[i-1] - f[i-2]) * 0.5f;
        // evaluate f at predictor
        float fPred = -pred + t[i];
        // corrector (AM2): y_i = y_{i-1} + h*(f_{i-1} + f̂)/2
        y[i] = y[i-1] + stepH * (f[i-1] + fPred) * 0.5f;
        // store new f
        f[i] = -y[i] + t[i];
        ++i;
    }

    // print results (every 150th point)
    for (int j = 0; j < steps; j += 150) {
        std::printf("t = %.4f, y = %.4f\n", t[j], y[j]);
    }
    return 0;
}
