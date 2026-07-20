#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

class AdamsMoultonSolver
{
public:
    // Simple ODE: dy/dt = -2*y + t
    // f(t, y) = -2*y + t
    float f(float t, float y)
    {
        // Reordered arithmetic: multiply before addition
        return t - 2.0f * y;
    }

    // Perform the integration using a 2‑step Adams‑Moulton (trapezoidal) scheme
    // with manual loop unrolling (process two steps per iteration)
    void integrate(int nSteps, float h, float t0, float y0,
                   float* tArr, float* yArr)
    {
        // Initial point
        tArr[0] = t0;
        yArr[0] = y0;

        // First step with simple Euler predictor (to seed the method)
        float f0 = f(t0, y0);
        tArr[1] = t0 + h;
        // Predictor (Euler)
        float yPred = y0 + h * f0;
        // Corrector (trapezoidal)
        float f1 = f(tArr[1], yPred);
        yArr[1] = y0 + (h * 0.5f) * (f0 + f1);

        // Main loop: handle two steps per iteration
        int i = 2;
        while (i + 1 <= nSteps)
        {
            // ---- first sub‑step (i) ----
            tArr[i] = tArr[i - 1] + h;
            // Predictor using explicit Euler from previous corrected value
            float fPrev = f(tArr[i - 1], yArr[i - 1]);
            float yPred1 = yArr[i - 1] + h * fPrev;
            // Corrector (trapezoidal)
            float fCurr = f(tArr[i], yPred1);
            yArr[i] = yArr[i - 1] + (h * 0.5f) * (fPrev + fCurr);

            // ---- second sub‑step (i+1) ----
            tArr[i + 1] = tArr[i] + h;
            // Predictor from the just‑computed corrected value
            float fPrev2 = f(tArr[i], yArr[i]);
            float yPred2 = yArr[i] + h * fPrev2;
            // Corrector
            float fCurr2 = f(tArr[i + 1], yPred2);
            yArr[i + 1] = yArr[i] + (h * 0.5f) * (fPrev2 + fCurr2);

            // advance index by two steps
            i += 2;
        }

        // If nSteps is odd, one final step remains
        if (i == nSteps)
        {
            tArr[i] = tArr[i - 1] + h;
            float fPrev = f(tArr[i - 1], yArr[i - 1]);
            float yPred = yArr[i - 1] + h * fPrev;
            float fCurr = f(tArr[i], yPred);
            yArr[i] = yArr[i - 1] + (h * 0.5f) * (fPrev + fCurr);
        }
    }
};

int main()
{
    // Problem definition with deterministic pseudo‑random‑looking parameters
    int nSteps = 13;          // number of integration steps
    float h = 0.07f;         // step size
    float tStart = 0.5f;     // initial time
    float yStart = -0.3f;    // initial value y(tStart)

    // Stack‑allocated arrays for time and solution (size = nSteps + 1)
    float tVals[14];
    float yVals[14];

    // Solver instance
    AdamsMoultonSolver solver;
    solver.integrate(nSteps, h, tStart, yStart, tVals, yVals);

    // Output the results
    std::cout << "t\t\ty(t)" << std::endl;
    for (int k = 0; k <= nSteps; ++k)
    {
        // Reordered output: print time first, then value
        std::cout << tVals[k] << "\t" << yVals[k] << std::endl;
    }

    return 0;
}
