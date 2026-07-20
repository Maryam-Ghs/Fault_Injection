/* LLM input variant 5: duplicate-heavy */
#include <iostream>
#include <cmath>   // only for reference output, not for computation

// ------------------------------------------------------------
// Version #10 – Adams–Bashforth explicit 2‑step solver
// ------------------------------------------------------------

class AdamsBashforthSolver {
public:
    // Constructor: set step count and step size
    AdamsBashforthSolver(int steps, float step_h) {
        totalSteps = steps;
        this->step_h = step_h;
        // Allocate vectors on the heap
        timeVec = new float[totalSteps + 1];
        solVec  = new float[totalSteps + 1];
        derivVec = new float[totalSteps + 1];
    }

    // Destructor: free heap memory
    ~AdamsBashforthSolver() {
        delete[] timeVec;
        delete[] solVec;
        delete[] derivVec;
    }

    // Right‑hand side of the ODE: dy/dt = -y
    float rhs(float t, float y) {
        // simple linear decay, no double used
        return -y;
    }

    // Perform the integration and print the trajectory
    void run() {
        // ---------- Initialise first point ----------
        int idx = 0;
        timeVec[idx] = 0.0f;
        solVec[idx]  = 1.0f;                 // y(0) = 1
        derivVec[idx] = rhs(timeVec[idx], solVec[idx]);

        // ---------- Compute second point with Euler ----------
        idx = 1;
        float h = step_h;
        float t_prev = timeVec[idx - 1];
        float y_prev = solVec[idx - 1];
        float f_prev = derivVec[idx - 1];

        // Euler step: y1 = y0 + h * f0
        float y_temp = y_prev + h * f_prev;
        timeVec[idx] = t_prev + h;
        solVec[idx]  = y_temp;
        derivVec[idx] = rhs(timeVec[idx], solVec[idx]);

        // ---------- Adams–Bashforth 2‑step loop ----------
        idx = 2;
        while (idx <= totalSteps) {
            // Gather needed previous data
            float t_nm1 = timeVec[idx - 1];
            float y_nm1 = solVec[idx - 1];
            float f_nm1 = derivVec[idx - 1];

            float t_nm2 = timeVec[idx - 2];
            float f_nm2 = derivVec[idx - 2];

            // Compute coefficients (reordered but mathematically same)
            float coeff1 = 1.5f * h;      // 3/2 * h
            float coeff2 = -0.5f * h;     // -1/2 * h

            // AB2 formula: y_{n+1} = y_n + h/2 * (3*f_n - f_{n-1})
            float incr = coeff1 * f_nm1 + coeff2 * f_nm2;
            float y_next = y_nm1 + incr;

            // Advance time
            float t_next = t_nm1 + h;

            // Store results
            timeVec[idx] = t_next;
            solVec[idx]  = y_next;
            derivVec[idx] = rhs(t_next, y_next);

            // Verbose output for this step
            std::cout << "Step " << idx << ": "
                      << "t = " << t_next << ", "
                      << "y = " << y_next << std::endl;

            // Move to next index
            idx = idx + 1;
        }
    }

private:
    int totalSteps;   // number of integration steps
    float step_h;      // uniform step size
    float *timeVec;   // array of time points
    float *solVec;    // array of solution values
    float *derivVec;  // array of derivative evaluations
};

int main() {
    // Deterministic test configuration with duplicate-heavy input
    int N = 8;                // total steps
    float h = 0.0f;           // step size (zero leads to repeated values)

    // Create solver instance on the stack
    AdamsBashforthSolver solver(N, h);

    // Run the method and print the trajectory
    solver.run();

    return 0;
}
