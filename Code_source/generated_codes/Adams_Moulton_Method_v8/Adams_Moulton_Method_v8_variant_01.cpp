/* LLM input variant 1: minimal-boundary */
// Adams–Moulton Method – version #8
// ------------------------------------------------------------
// Implements the 2‑step implicit Adams‑Moulton (trapezoidal) scheme
// for the stiff ODE  y' = -1000*y + sin(t)  using only float/int.
// ------------------------------------------------------------

#include <cstdio>
#include <cmath>   // only for std::sin, std::fabs

// ------------------------------------------------------------
// ODE definition – can be changed to test other edge cases
// ------------------------------------------------------------
float rhs(float t, float y) {
    // Stiff term with large magnitude, plus a mild sinusoidal forcing
    return -1000.0f * y + std::sin(t);
}

// ------------------------------------------------------------
// Class that encapsulates the Adams‑Moulton solver
// ------------------------------------------------------------
class AdamsMoulton {
public:
    // Constructor – sets up the problem size and step
    AdamsMoulton(int steps, float step) {
        nSteps = steps;
        h = step;

        // Allocate arrays on the heap
        tVals = new float[nSteps + 1];
        yVals = new float[nSteps + 1];
    }

    // Destructor – frees heap memory
    ~AdamsMoulton() {
        delete[] tVals;
        delete[] yVals;
    }

    // Main driver – solves the IVP and prints the trajectory
    void solve(float t0, float y0) {
        // Initialise first point
        tVals[0] = t0;
        yVals[0] = y0;

        // Use explicit Euler for the first step (predictor)
        tVals[1] = t0 + h;
        yVals[1] = y0 + h * rhs(t0, y0);

        // Main integration loop (loop‑heavy, iterative)
        int i = 1;
        while (i < nSteps) {
            // Predict using Adams‑Bashforth 2‑step
            float tPred = tVals[i] + h;
            float f_i   = rhs(tVals[i],   yVals[i]);
            float f_im1 = rhs(tVals[i-1], yVals[i-1]);
            float yPred = yVals[i] + h * (1.5f * f_i - 0.5f * f_im1);

            // Correct with Adams‑Moulton 2‑step (fixed‑point iteration)
            float yCorr = yPred;                 // start from predictor
            int    iter  = 0;
            const int maxIter = 20;
            while (iter < maxIter) {
                float f_new = rhs(tPred, yCorr);
                float yNew  = yVals[i] + (h * 0.5f) * (f_new + f_i);
                // convergence test (tolerant for float)
                if (std::fabs(yNew - yCorr) < 1e-5f) break;
                yCorr = yNew;
                ++iter;
            }

            // Store the accepted value
            tVals[i+1] = tPred;
            yVals[i+1] = yCorr;

            ++i;   // advance index
        }

        // Output the result table
        int j = 0;
        while (j <= nSteps) {
            std::printf("%8.4f %12.6f\n", tVals[j], yVals[j]);
            ++j;
        }
    }

private:
    int   nSteps;   // number of integration steps
    float h;        // step size
    float* tVals;   // time grid (heap allocated)
    float* yVals;   // solution grid (heap allocated)
};

// ------------------------------------------------------------
// Entry point – generates a challenging test case internally
// ------------------------------------------------------------
int main() {
    // Minimal‑boundary parameters
    const int    totalSteps = 1;          // smallest non‑trivial number of steps
    const float  stepSize   = 1e-6f;       // smallest positive step size
    const float  startTime  = 0.0f;
    const float  startVal   = 0.0f;       // equilibrium start value (still non‑trivial due to forcing)

    // Instantiate solver and run
    AdamsMoulton solver(totalSteps, stepSize);
    solver.solve(startTime, startVal);

    return 0;
}
