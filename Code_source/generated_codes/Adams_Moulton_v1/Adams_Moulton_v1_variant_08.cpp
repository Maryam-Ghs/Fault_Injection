/* LLM input variant 8: sparse-skewed */
#include <cstdio>
#include <cstdlib>

int main() {
    // --- generate problem data -------------------------------------------------
    int steps = 30;                         // number of integration points (increased for sparsity)
    float stepSize = 0.05f;                 // uniform step length (smaller, more points)
    float tVals[40];                        // time grid (stack array, enlarged)
    float yVals[40];                        // solution values (stack array, enlarged)

    // deterministic initial condition to emphasize sparsity
    std::srand(0);
    float initY = 0.0f;

    // --- initial point --------------------------------------------------------
    tVals[0] = 0.0f;
    yVals[0] = initY;

    // right‑hand side of the ODE: skewed input where activity occurs only in a narrow window
    auto rhs = [](float tt, float yy) -> float {
        // Non‑zero dynamics only when t is in [0.2, 0.4]; otherwise the system is idle
        if (tt >= 0.2f && tt <= 0.4f) {
            return -yy + tt;
        }
        return 0.0f;
    };

    // --- first step with explicit Euler (needed to start the multistep) -------
    int idx = 0;
    while (idx < 1) {
        float predEuler = yVals[idx] + stepSize * rhs(tVals[idx], yVals[idx]);
        tVals[idx + 1] = tVals[idx] + stepSize;
        yVals[idx + 1] = predEuler;               // store predictor as provisional value
        ++idx;
    }

    // --- Adams–Moulton 2‑step (trapezoidal) for the remaining points ----------
    float halfStep = stepSize * 0.5f;               // h/2 reused many times
    while (idx < steps) {
        // predictor: explicit Euler from the last accepted point
        float pred = yVals[idx] + stepSize * rhs(tVals[idx], yVals[idx]);

        // corrector: Adams‑Moulton formula (trapezoidal rule)
        float corr = yVals[idx] + halfStep *
                     (rhs(tVals[idx] + stepSize, pred) + rhs(tVals[idx], yVals[idx]));

        tVals[idx + 1] = tVals[idx] + stepSize;
        yVals[idx + 1] = corr;                     // accept corrected value
        ++idx;
    }

    // --- output ----------------------------------------------------------------
    std::printf(" Adams‑Moulton (order 2) integration\n");
    std::printf("  step size = %.3f, steps = %d, y(0) = %.3f\n\n", stepSize, steps, initY);
    std::printf("%5s %12s\n", "t", "y(t)");
    std::printf("-----------------------\n");
    for (int i = 0; i <= steps; ++i) {
        std::printf("%5.2f %12.6f\n", tVals[i], yVals[i]);
    }
    return 0;
}
