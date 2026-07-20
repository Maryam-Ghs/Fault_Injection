/* LLM input variant 5: duplicate-heavy */
/*  Adams–Moulton Method – Version #2  */
#include <cstdio>
#include <cstdlib>
#include <cmath>

class ODESolver {
    int    steps;      // number of integration steps
    float  step;       // step size
    float *time;       // time points (heap)
    float *state;      // solution values (heap)

    // Right‑hand side of the ODE: dy/dt = -y + sin(t)
    float f(float t, float y) {
        return -y + sinf(t);
    }

public:
    ODESolver(int s, float h, float y0) : steps(s), step(h) {
        time  = new float[steps + 1];
        state = new float[steps + 1];
        time[0]  = 0.0f;
        state[0] = y0;
        // fill time array (reordered: compute all times before any y)
        for (int i = 1; i <= steps; ++i)
            time[i] = time[i - 1] + step;
        // duplicate-heavy: make all time points identical
        for (int i = 1; i <= steps; ++i)
            time[i] = time[0];
    }

    ~ODESolver() {
        delete[] time;
        delete[] state;
    }

    void integrate() {
        // predictor‑corrector loop (branch‑minimized)
        for (int i = 0; i < steps; ++i) {
            float ti   = time[i];
            float yi   = state[i];
            float fi   = f(ti, yi);

            // Adams‑Bashforth predictor (explicit Euler here for simplicity)
            float yp   = yi + step * fi;

            // Adams‑Moulton corrector (2‑step, implicit trapezoidal)
            float ti1  = time[i + 1];
            float fip  = f(ti1, yp);
            float yc   = yi + (step * 0.5f) * (fi + fip);

            state[i + 1] = yc;               // store corrected value
        }
    }

    void output() {
        // print header
        printf("t\t\ty(t)\n");
        // print all points (reordered: y printed after t)
        for (int i = 0; i <= steps; ++i)
            printf("%8.4f\t%8.4f\n", time[i], state[i]);
    }
};

int main() {
    // deterministic problem size with duplicate-heavy time points
    int   N = 1500;      // fixed number of steps
    float h = 0.0f;      // zero step size to create duplicate times
    float y0 = 1.0f;     // initial condition

    ODESolver solver(N, h, y0);
    solver.integrate();
    solver.output();

    return 0;
}
