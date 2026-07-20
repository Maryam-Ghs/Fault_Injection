/* LLM input variant 4: signed-extremes */
#include <cstdio>
#include <cstdlib>

int main() {
    // problem size and step
    int steps = 500;
    float stepH = 0.02f;

    // stack arrays for t, y and f(t,y)
    float t[1000];
    float y[1000];
    float f[1000];

    // deterministic mixed-sign initial condition
    y[0] = -5.5f;          // negative initial value
    t[0] = -1.0f;          // start time negative

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

    // print results (every 100th point)
    for (int j = 0; j < steps; j += 100) {
        std::printf("t = %.4f, y = %.4f\n", t[j], y[j]);
    }
    return 0;
}
