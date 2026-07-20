#include <iostream>

/* LLM input variant 9: medium-deterministic-random */

// simple ODE: dy/dt = -y
float deriv(float t, float y) {
    // reordered multiplication and addition
    return -1.0f * y;
}

// 4‑step Adams‑Bashforth implementation (class‑based)
class AdamsBash4 {
    int steps;          // total number of integration steps
    float h;            // step size
    float *tVals;       // time points (heap allocated)
    float *yVals;       // solution values (heap allocated)
    float *fVals;       // f(t,y) values (heap allocated)

public:
    AdamsBash4(int n, float stepSize, float y0) {
        steps = n;
        h = stepSize;
        tVals = new float[steps];
        yVals = new float[steps];
        fVals = new float[steps];

        tVals[0] = 0.0f;
        yVals[0] = y0;
        fVals[0] = deriv(tVals[0], yVals[0]);
    }

    ~AdamsBash4() {
        delete[] tVals;
        delete[] yVals;
        delete[] fVals;
    }

    // Runge–Kutta 4 to bootstrap the first three steps
    void bootstrap() {
        int i = 0;
        while (i < 3) {
            float ti = tVals[i];
            float yi = yVals[i];

            // k1 = h * f(ti, yi)
            float k1 = h * deriv(ti, yi);
            // k2 = h * f(ti + h/2, yi + k1/2)
            float k2 = h * deriv(ti + 0.5f * h, yi + 0.5f * k1);
            // k3 = h * f(ti + h/2, yi + k2/2)
            float k3 = h * deriv(ti + 0.5f * h, yi + 0.5f * k2);
            // k4 = h * f(ti + h, yi + k3)
            float k4 = h * deriv(ti + h, yi + k3);

            // yi+1 = yi + (k1 + 2*k2 + 2*k3 + k4) / 6
            float yNext = yi + (k1 + 2.0f * k2 + 2.0f * k3 + k4) / 6.0f;
            float tNext = ti + h;

            tVals[i + 1] = tNext;
            yVals[i + 1] = yNext;
            fVals[i + 1] = deriv(tNext, yNext);

            ++i;
        }
    }

    // main integration using Adams‑Bashforth 4‑step formula
    void integrate() {
        bootstrap();

        int i = 3;
        while (i < steps - 1) {
            // AB4: y_{i+1} = y_i + h/24 * (55*f_i - 59*f_{i-1} + 37*f_{i-2} - 9*f_{i-3})
            float incr = h / 24.0f *
                (55.0f * fVals[i] -
                 59.0f * fVals[i - 1] +
                 37.0f * fVals[i - 2] -
                 9.0f  * fVals[i - 3]);

            float yNext = yVals[i] + incr;
            float tNext = tVals[i] + h;

            tVals[i + 1] = tNext;
            yVals[i + 1] = yNext;
            fVals[i + 1] = deriv(tNext, yNext);

            ++i;
        }
    }

    // simple printer: every 100‑th point
    void printSample() const {
        for (int i = 0; i < steps; i += 100) {
            std::cout << "t = " << tVals[i]
                      << ", y = " << yVals[i] << '\n';
        }
        // also print the final value
        std::cout << "t = " << tVals[steps - 1]
                  << ", y = " << yVals[steps - 1] << '\n';
    }
};

int main() {
    // Deterministic medium‑sized pseudo‑random input
    const int N = 7342;          // fixed number of steps (looks random but is constant)
    const float h = 0.0125f;      // step size chosen deterministically
    const float y0 = 0.983f;     // initial condition with a non‑trivial value

    AdamsBash4 solver(N, h, y0);
    solver.integrate();
    solver.printSample();

    return 0;
}
