/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <cmath>   // only for the reference solution (optional)

// ------------------------------------------------------------
// Class implementing a 2‑step Adams‑Bashforth integrator
// ------------------------------------------------------------
class AdamsBash {
public:
    // Constructor: allocate storage for the solution vectors
    AdamsBash(int totalSteps, float stepSize) {
        N      = totalSteps;
        h      = stepSize;
        // heap allocation as required
        tVals  = new float[N + 1];
        yVals  = new float[N + 1];
        fHist  = new float[2];   // store the two previous f‑values
    }

    // Destructor: release heap memory
    ~AdamsBash() {
        delete[] tVals;
        delete[] yVals;
        delete[] fHist;
    }

    // Public interface: run the integration and print the table
    void run() {
        initialise();
        // ---- main loop (while instead of for) ----
        int pos = 2;
        while (pos <= N) {
            // fused expression for the Adams‑Bashforth predictor
            float predY = yVals[pos-1] + h * ( (3.0f/2.0f) * fHist[1] - (0.5f) * fHist[0] );

            // advance time (reordered: compute new time before storing it)
            float newT = tVals[pos-1] + h;
            tVals[pos] = newT;

            // evaluate the derivative at the predicted point
            float newF = rhs(newT, predY);
            yVals[pos] = predY;            // store the predicted y

            // shift the history buffer for the next step
            fHist[0] = fHist[1];
            fHist[1] = newF;

            ++pos;
        }
        output();
    }

private:
    int   N;          // number of integration steps
    float h;          // step size
    float *tVals;     // time array (heap)
    float *yVals;     // solution array (heap)
    float *fHist;     // last two derivative values

    // Right‑hand side of the ODE: y' = -y
    float rhs(float t, float y) {
        (void)t;                // t is unused for this simple ODE
        return -y;
    }

    // Simple Euler step to generate the first two points
    void initialise() {
        // initial condition
        tVals[0] = 0.0f;
        yVals[0] = 1.0f;
        fHist[0] = rhs(tVals[0], yVals[0]);

        // first Euler step
        tVals[1] = tVals[0] + h;
        yVals[1] = yVals[0] + h * fHist[0];
        fHist[1] = rhs(tVals[1], yVals[1]);
    }

    // Print the results in a table
    void output() {
        std::cout << "Step\t t\t y (AB2)\n";
        for (int i = 0; i <= N; ++i) {
            std::cout << i << "\t " << tVals[i] << "\t " << yVals[i] << "\n";
        }
    }
};

// ------------------------------------------------------------
// Entry point
// ------------------------------------------------------------
int main() {
    // minimal predefined configuration (version #3)
    int   steps   = 2;           // smallest non‑trivial number of steps for 2‑step AB
    float stepSz  = 0.5f;       // boundary‑valid step size

    // instantiate the solver (class‑based, heap‑allocated data)
    AdamsBash solver(steps, stepSz);
    solver.run();                 // perform integration and print

    return 0;
}
