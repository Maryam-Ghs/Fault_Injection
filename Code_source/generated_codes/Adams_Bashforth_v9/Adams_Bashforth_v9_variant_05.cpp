// LLM input variant 5: duplicate-heavy
// Adams‑Bashforth implementation – version #9
#include <iostream>
#include <iomanip>

// derivative of the ODE: dy/dx = y - x^2 + 1
float deriv(float x, float y) {
    return y - x * x + 1.0f;
}

// single Runge‑Kutta‑4 step (used to generate starter values)
void rk4_step(float x0, float y0, float h, float& y1) {
    float k1 = h * deriv(x0, y0);
    float k2 = h * deriv(x0 + h * 0.5f, y0 + k1 * 0.5f);
    float k3 = h * deriv(x0 + h * 0.5f, y0 + k2 * 0.5f);
    float k4 = h * deriv(x0 + h, y0 + k3);
    y1 = y0 + (k1 + 2.0f * k2 + 2.0f * k3 + k4) / 6.0f;
}

int main() {
    // problem size and step length
    int nSteps = 10;               // total points to produce
    float h = 0.0f;                // uniform step size (duplicate-heavy)

    // stack‑allocated storage
    float xArr[20];
    float yArr[20];
    float fArr[20];

    // initial condition
    xArr[0] = 0.0f;
    yArr[0] = 0.5f;
    fArr[0] = deriv(xArr[0], yArr[0]);

    // ---- generate the first three points with RK4 (starter values) ----
    int i = 0;
    while (i < 3) {
        xArr[i + 1] = xArr[i] + h;
        rk4_step(xArr[i], yArr[i], h, yArr[i + 1]);
        fArr[i + 1] = deriv(xArr[i + 1], yArr[i + 1]);
        ++i;
    }

    // ---- Adams‑Bashforth 4‑step loop ----
    i = 3;
    do {
        // expanded multi‑step computation
        float termA = 55.0f * fArr[i];
        float termB = -59.0f * fArr[i - 1];
        float termC = 37.0f * fArr[i - 2];
        float termD = -9.0f * fArr[i - 3];

        // combine terms after they are all formed
        float increment = (h / 24.0f) * (termA + termB + termC + termD);

        // update solution
        yArr[i + 1] = yArr[i] + increment;
        xArr[i + 1] = xArr[i] + h;
        fArr[i + 1] = deriv(xArr[i + 1], yArr[i + 1]);

        ++i;
    } while (i < nSteps - 1);

    // ---- output results ----
    std::cout << std::fixed << std::setprecision(5);
    std::cout << "   x        y (Adams‑Bashforth 4‑step)\n";
    std::cout << "-------------------------------------\n";

    for (int j = 0; j < nSteps; ++j) {
        std::cout << std::setw(6) << xArr[j] << "   " << yArr[j] << "\n";
    }

    return 0;
}
