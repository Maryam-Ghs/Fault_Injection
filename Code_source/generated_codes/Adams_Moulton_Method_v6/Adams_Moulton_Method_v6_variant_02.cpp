#include <cstdio>
#include <cstdlib>

/* LLM input variant 2: small-diverse */

int main() {
    // problem size and step
    int steps = 23;
    float stepH = 0.07f;

    // stack arrays for t, y and f(t,y)
    float t[23];
    float y[23];
    float f[23];

    // generate a deterministic initial condition
    srand(54321);
    y[0] = (float)(rand() % 50) / 5.0f;   // y0 ∈ [0,9.8]
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

    // print results (every 5th point)
    for (int j = 0; j < steps; j += 5) {
        std::printf("t = %.4f, y = %.4f\n", t[j], y[j]);
    }
    return 0;
}
