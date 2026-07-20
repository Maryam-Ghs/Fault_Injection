/* LLM input variant 2: small-diverse */
#include <iostream>
#include <iomanip>

int main() {
    // ------------ parameters (all on stack) ------------
    int stepCount = 12;          // total number of points (reduced for small test)
    float stepSize = 0.15f;       // time step h (different value)

    // ------------ storage arrays ------------
    float tVals[12];
    float yVals[12];
    float fVals[12];

    // ------------ initial condition ------------
    yVals[0] = 2.0f;             // y(0) = 2 (varied initial value)
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
