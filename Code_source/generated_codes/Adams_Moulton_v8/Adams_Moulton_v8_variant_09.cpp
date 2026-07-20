/* LLM input variant 9: medium-deterministic-random */
// Adams‑Moulton ODE solver – version 8
//  ------------------------------------------------------------
//  Implements a 2‑step Adams‑Moulton corrector (order 2) on
//  a fixed‑step grid using only float and int types.
//  Input data are generated internally as large random arrays.
//  ------------------------------------------------------------

#include <cstdio>
#include <cstdlib>

class AdamsMoulton {
public:
    // Solve dy/dt = -p*y + q*sin(r*t) on [0, T] with N steps
    // Results are stored in yArr (size N+1) and printed.
    void solve(int N, float h, float y0, float p, float q, float r) {
        // Stack‑allocated buffers (size limited by N)
        float tArr[2001];
        float yArr[2001];
        float fArr[2001];

        // Initialise first point
        tArr[0] = 0.0f;
        yArr[0] = y0;
        fArr[0] = deriv(tArr[0], yArr[0], p, q, r);

        // First step – explicit Euler to bootstrap multistep scheme
        tArr[1] = tArr[0] + h;
        yArr[1] = yArr[0] + h * fArr[0];
        fArr[1] = deriv(tArr[1], yArr[1], p, q, r);

        // Main integration loop (branch‑minimized, fused expressions)
        int i = 1;
        while (i < N) {
            // Predict y_{i+1} using 2‑step Adams‑Bashforth (explicit)
            float yPred = yArr[i] + h * (1.5f * fArr[i] - 0.5f * fArr[i - 1]);

            // Advance time
            tArr[i + 1] = tArr[i] + h;

            // Evaluate derivative at predicted point
            float fPred = deriv(tArr[i + 1], yPred, p, q, r);

            // Correct y_{i+1} with 2‑step Adams‑Moulton (implicit, using prediction)
            yArr[i + 1] = yArr[i] + h * (0.5f * fPred + 0.5f * fArr[i]);

            // Store corrected derivative for next step
            fArr[i + 1] = deriv(tArr[i + 1], yArr[i + 1], p, q, r);

            ++i;   // loop progression – no extra branching
        }

        // Output the solution (time, value) for verification
        for (i = 0; i <= N; ++i) {
            std::printf("%8.4f %12.6f\n", tArr[i], yArr[i]);
        }
    }

private:
    // Right‑hand side of the ODE (inline for speed, fused ops)
    float deriv(float t, float y, float p, float q, float r) {
        // dy/dt = -p*y + q*sin(r*t)
        // Using std::sin requires double; implement a simple approx with Taylor (branch‑free)
        // sin(x) ≈ x - x³/6 + x⁵/120  (good enough for demonstration)
        float x = r * t;
        float x2 = x * x;
        float sinx = x - (x2 * x) * (1.0f / 6.0f) + (x2 * x2 * x) * (1.0f / 120.0f);
        return -p * y + q * sinx;
    }
};

// Simple deterministic pseudo‑random generator (LCG) with fixed seed
static unsigned int lcg_state = 123456789u;
unsigned int lcg_next() {
    lcg_state = lcg_state * 1103515245u + 12345u;
    return (lcg_state >> 16) & 0x7FFFu;
}

int main() {
    // Deterministic problem size (large array) – up to 2000 steps
    int steps = 1500 + (lcg_next() % 501);          // 1500 … 2000
    float stepSize = 0.01f;                         // fixed step

    // Deterministic initial condition and parameters
    float yInit = -10.0f + 20.0f * (lcg_next() / (float)0x7FFF);
    float p = 0.5f + 1.5f * (lcg_next() / (float)0x7FFF);
    float q = 1.0f + 4.0f * (lcg_next() / (float)0x7FFF);
    float r = 0.5f + 2.0f * (lcg_next() / (float)0x7FFF);

    AdamsMoulton solver;
    solver.solve(steps, stepSize, yInit, p, q, r);

    return 0;
}
