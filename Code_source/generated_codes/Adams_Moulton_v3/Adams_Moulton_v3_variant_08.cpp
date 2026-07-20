#include <cstdio>
#include <cmath>

/* LLM input variant 8: sparse-skewed */
int main() {
    // ---- version #3 ----
    // Edge‑case heavy parameters
    int steps = 50000;                 // many steps
    float h = 0.0005f;                 // very small step size
    // Allocate arrays on the heap
    float* timeArr = new float[steps + 1];
    float* solArr  = new float[steps + 1];

    // ODE: dy/dt = -y  (exact solution y = exp(-t))
    auto f = [](float t, float y) -> float { return -y; };

    // Initial condition
    timeArr[0] = 0.0f;
    solArr[0]  = 1.0f;                 // y(0) = 1

    // Fill time array with a sparse‑skewed pattern
    for (int i = 0; i < steps; ++i) {
        // Every 1000th step makes a larger jump, creating sparsity and clustering
        float increment = (i % 1000 == 0) ? h * 1000.0f : h;
        float tmpT = timeArr[i] + increment;
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
        float exact = expf(-timeArr[i]);
        float diff  = solArr[i] - exact;
        float absDiff = fabsf(diff);
        if (absDiff > maxErr) maxErr = absDiff;
    }

    // Print final results
    printf("Final t = %f\n", timeArr[steps]);
    printf("Approx y = %f\n", solArr[steps]);
    printf("Exact y  = %f\n", expf(-timeArr[steps]));
    printf("Max error over interval = %f\n", maxErr);

    // Clean up heap memory
    delete[] timeArr;
    delete[] solArr;
    return 0;
}
