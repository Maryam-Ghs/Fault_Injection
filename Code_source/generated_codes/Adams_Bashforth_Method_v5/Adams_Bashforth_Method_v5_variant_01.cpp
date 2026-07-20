#include <iostream>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */
int main() {
    // ------------ parameters (all on stack) ------------
    int stepCount = 4;           // minimal total number of points
    float stepSize = 0.1f;       // smallest non‑zero time step

    // ------------ storage arrays ------------
    float tVals[4];
    float yVals[4];
    float fVals[4];

    // ------------ initial condition ------------
    yVals[0] = 1.0f;             // y(0) = 1
    tVals[0] = 0.0f;             // t0
    fVals[0] = -yVals[0];        // f = dy/dt = -y

    // ------------ seed first three points with RK4 ------------
    int idx = 0;
    while (idx < 3) {
        // k1
        float k1 = fVals[idx];

        // k2
        float yTmp = yVals[idx] + 0.5f * stepSize * k1;
        float k2 = -yTmp;

        // k3
        yTmp = yVals[idx] + 0.5f * stepSize * k2;
        float k3 = -yTmp;

        // k4
        yTmp = yVals[idx] + stepSize * k3;
        float k4 = -yTmp;

        // next y
        yVals[idx + 1] = yVals[idx] + (stepSize / 6.0f) * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
        tVals[idx + 1] = tVals[idx] + stepSize;
        fVals[idx + 1] = -yVals[idx + 1];

        ++idx;
    }

    // ------------ Adams‑Bashforth 4‑step loop ------------
    while (idx < stepCount - 1) {
        // compute increment using the 4‑step AB formula
        float incr = (stepSize / 24.0f) *
                     (55.0f * fVals[idx]
                      - 59.0f * fVals[idx - 1]
                      + 37.0f * fVals[idx - 2]
                      - 9.0f * fVals[idx - 3]);

        // advance solution
        yVals[idx + 1] = yVals[idx] + incr;
        tVals[idx + 1] = tVals[idx] + stepSize;
        fVals[idx + 1] = -yVals[idx + 1];

        ++idx;
    }

    // ------------ output results ------------
    for (int i = 0; i < stepCount; ++i) {
        std::cout << std::fixed << std::setprecision(6)
                  << tVals[i] << " " << yVals[i] << "\n";
    }

    return 0;
}
