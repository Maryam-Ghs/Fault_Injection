#include <cstdio>
#include <cstdlib>

/* LLM input variant 10: large-safe-stress */
int main() {
    // --- generate problem data -------------------------------------------------
    const int steps = 1000;                   // number of integration points (large but safe)
    const float stepSize = 0.01f;             // smaller uniform step length for finer resolution
    float tVals[2005];                        // time grid (stack array, sized for safety)
    float yVals[2005];                        // solution values (stack array)

    // random initial condition in [-1,1]
    std::srand(0);
    float initY = static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f;

    // --- initial point --------------------------------------------------------
    tVals[0] = 0.0f;
    yVals[0] = initY;

    // right‑hand side of the ODE: y' = -y + t
    auto rhs = [](float tt, float yy) -> float {
        return -yy + tt;
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
    const float halfStep = stepSize * 0.5f;               // h/2 reused many times
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
