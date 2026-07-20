#include <cstdio>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // ---- version #3 ----
    // Medium‑size deterministic pseudo‑random‑looking parameters
    int steps = 12345;                 // medium number of steps
    float h = 0.0012345f;              // deterministic small step size

    // Allocate arrays on the heap
    float* timeArr = new float[steps + 1];
    float* solArr  = new float[steps + 1];

    // ODE: dy/dt = -y  (exact solution y = y0 * exp(-t))
    auto f = [](float t, float y) -> float { return -y; };

    // Initial condition (deterministic non‑trivial value)
    timeArr[0] = 0.0f;
    solArr[0]  = 0.987654f;            // y(0) = 0.987654

    // Fill time array (loop‑heavy)
    for (int i = 0; i < steps; ++i) {
        float tmpT = timeArr[i] + h;   // split into temporary
        timeArr[i + 1] = tmpT;
    }

    // First step – explicit Euler to seed the multistep method
    {
        float t0   = timeArr[0];
        float y0   = solArr[0];
        float f0   = f(t0, y0);
        float y1   = y0 + h * f0;       // predictor
        solArr[1] = y1;
    }

    // Adams‑Moulton 2‑step (trapezoidal) loop
    for (int n = 1; n < steps; ++n) {
        // ----- Predictor: Adams‑Bashforth 2 -----
        float tn   = timeArr[n];
        float yn   = solArr[n];
        float tn_1 = timeArr[n - 1];
        float yn_1 = solArr[n - 1];

        float fn   = f(tn,   yn);
        float fn_1 = f(tn_1, yn_1);

        // predictor formula split into temporaries
        float predCoeff = 1.5f * fn - 0.5f * fn_1;
        float yPred    = yn + h * predCoeff;

        // ----- Corrector: Adams‑Moulton 2 (trapezoidal) -----
        float tn1   = timeArr[n + 1];
        float fnp1  = f(tn1, yPred);
        float corr  = (h * 0.5f) * (fn + fnp1);
        float yCorr = yn + corr;

        solArr[n + 1] = yCorr;          // store corrected value
    }

    // Compute maximum absolute error (additional loop)
    float maxErr = 0.0f;
    for (int i = 0; i <= steps; ++i) {
        float exact = solArr[0] * expf(-timeArr[i]); // exact solution with given y0
        float diff  = solArr[i] - exact;
        float absDiff = fabsf(diff);
        if (absDiff > maxErr) maxErr = absDiff;
    }

    // Print final results
    printf("Final t = %f\n", timeArr[steps]);
    printf("Approx y = %f\n", solArr[steps]);
    printf("Exact y  = %f\n", solArr[0] * expf(-timeArr[steps]));
    printf("Max error over interval = %f\n", maxErr);

    // Clean up heap memory
    delete[] timeArr;
    delete[] solArr;
    return 0;
}
