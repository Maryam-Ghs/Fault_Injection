#include <cstdio>

/* LLM input variant 6: ordered-structured */
// ------------------------------------------------------------
// Class implementing the 4‑step Adams‑Bashforth method
// ------------------------------------------------------------
class AdamsBashforthSolver {
public:
    // public members (no const allowed)
    int   numSteps;   // total number of integration steps
    float stepSize;   // integration step h
    float startTime;  // initial time t0
    float startValue; // initial solution y0

    // ----------------------------------------------------------------
    // Constructor – stores the integration parameters
    // ----------------------------------------------------------------
    AdamsBashforthSolver(int steps, float h, float t0, float y0) {
        numSteps  = steps;
        stepSize  = h;
        startTime = t0;
        startValue= y0;
    }

    // ----------------------------------------------------------------
    // Right‑hand side of the ODE: y' = -y
    // ----------------------------------------------------------------
    float rhs(float t, float y) {
        // simple linear decay, works for any t, y (including y = 0)
        return -y;
    }

    // ----------------------------------------------------------------
    // Main driver – performs the integration and prints the table
    // ----------------------------------------------------------------
    void solve() {
        // --------------------------------------------------------
        // Allocate fixed‑size stack arrays (enough for modest N)
        // --------------------------------------------------------
        const int MAXN = 100;                     // safe upper bound
        float   solution[MAXN + 1];
        float   moments  [MAXN + 1];
        float   derivs   [4];                     // last four f values

        // --------------------------------------------------------
        // Initialise first point
        // --------------------------------------------------------
        solution[0] = startValue;
        moments [0] = startTime;

        // --------------------------------------------------------
        // Bootstrap the first three steps with RK4 (explicit)
        // --------------------------------------------------------
        int i;
        for (i = 0; i < 3; ++i) {
            float ti = moments[i];
            float yi = solution[i];

            // ----- RK4 stage values -----
            float k1 = rhs(ti, yi);
            float k2 = rhs(ti + stepSize * 0.5f,
                           yi + stepSize * 0.5f * k1);
            float k3 = rhs(ti + stepSize * 0.5f,
                           yi + stepSize * 0.5f * k2);
            float k4 = rhs(ti + stepSize,
                           yi + stepSize * k3);

            // ----- combine stages -----
            float incr = stepSize *
                         (k1 + 2.0f * k2 + 2.0f * k3 + k4) / 6.0f;

            // ----- advance -----
            solution[i + 1] = yi + incr;
            moments [i + 1] = ti + stepSize;
        }

        // --------------------------------------------------------
        // Fill the derivative buffer with the first four f values
        // --------------------------------------------------------
        int j;
        for (j = 0; j < 4; ++j) {
            derivs[j] = rhs(moments[j], solution[j]);
        }

        // --------------------------------------------------------
        // Main Adams‑Bashforth 4‑step loop
        // --------------------------------------------------------
        for (i = 3; i < numSteps; ++i) {
            // ----- compute the next solution value -----
            float termA = 55.0f * derivs[3];
            float termB = -59.0f * derivs[2];
            float termC = 37.0f * derivs[1];
            float termD = -9.0f * derivs[0];

            float yNext = solution[i] +
                          stepSize * (termA + termB + termC + termD) / 24.0f;
            float tNext = moments[i] + stepSize;

            // ----- shift derivative buffer left -----
            derivs[0] = derivs[1];
            derivs[1] = derivs[2];
            derivs[2] = derivs[3];
            // ----- compute new derivative for the just‑computed point -----
            derivs[3] = rhs(tNext, yNext);

            // ----- store results -----
            solution[i + 1] = yNext;
            moments [i + 1] = tNext;
        }

        // --------------------------------------------------------
        // Print the table: time and numerical solution
        // --------------------------------------------------------
        for (i = 0; i <= numSteps; ++i) {
            std::printf("t = %6.3f , y = %12.6f\n",
                        moments[i], solution[i]);
        }
    }
};

// ------------------------------------------------------------
// Entry point – generate a highly structured set of test cases
// ------------------------------------------------------------
int main() {
    // Structured input: a sorted list of initial values spanning negative to positive
    const float initValues[] = { -1.0f, -0.5f, 0.0f, 0.5f, 1.0f };
    const int   numInits    = sizeof(initValues) / sizeof(initValues[0]);

    const int    totalSteps = 20;   // moderate number for clear output
    const float  h          = 0.05f; // regular step size
    const float  t0         = -1.0f; // start time (ordered, symmetric around zero)

    for (int idx = 0; idx < numInits; ++idx) {
        std::printf("=== Run %d: y0 = % .2f ===\n", idx + 1, initValues[idx]);
        AdamsBashforthSolver solver(totalSteps, h, t0, initValues[idx]);
        solver.solve();
        std::printf("\n");
    }

    return 0;
}
