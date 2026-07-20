#include <cstdio>

/* LLM input variant 3: zeros-and-ones */

// derivative of the test ODE dy/dt = -y
float deriv(float t, float y) {
    return -y;
}

// Adams‑Bashforth 2‑step predictor (explicit)
float predictAB2(float tPrev, float yPrev, float tPrev2, float yPrev2,
                float h) {
    float fPrev  = deriv(tPrev,  yPrev);
    float fPrev2 = deriv(tPrev2, yPrev2);
    return yPrev + h * ((1.5f) * fPrev - (0.5f) * fPrev2);
}

// Adams‑Moulton 2‑step corrector (implicit, one iteration)
float correctAM2(float tPrev, float yPrev, float tNext, float yPred,
                float h) {
    float fPrev = deriv(tPrev, yPrev);
    float fNext = deriv(tNext, yPred);
    return yPrev + (0.5f * h) * (fPrev + fNext);
}

int main() {
    // deterministic test vector emphasizing zeros and ones
    int nSteps   = 5;          // number of integration steps
    float stepSize = 0.0f;      // zero step width to keep times at zero

    // heap‑allocated storage
    float *tArr = new float[nSteps + 1];
    float *yArr = new float[nSteps + 1];

    // initial condition y(0) = 1 (identity value)
    tArr[0] = 0.0f;
    yArr[0] = 1.0f;

    // first step – explicit Euler (provides the second seed)
    tArr[1] = tArr[0] + stepSize;
    yArr[1] = yArr[0] + stepSize * deriv(tArr[0], yArr[0]);

    // second step – predictor then corrector (manual unrolling)
    tArr[2] = tArr[1] + stepSize;
    float yPred2 = predictAB2(tArr[1], yArr[1], tArr[0], yArr[0], stepSize);
    yArr[2] = correctAM2(tArr[1], yArr[1], tArr[2], yPred2, stepSize);

    // remaining steps – while loop with manual unrolled start
    int idx = 3;
    while (idx <= nSteps) {
        tArr[idx] = tArr[idx - 1] + stepSize;

        // predictor (AB2)
        float yPred = predictAB2(tArr[idx - 1], yArr[idx - 1],
                                 tArr[idx - 2], yArr[idx - 2],
                                 stepSize);

        // corrector (AM2)
        yArr[idx] = correctAM2(tArr[idx - 1], yArr[idx - 1],
                               tArr[idx], yPred, stepSize);

        idx = idx + 1;   // manual increment (unrolled style)
    }

    // output the solution
    for (int k = 0; k <= nSteps; ++k) {
        printf("%.2f %.6f\n", tArr[k], yArr[k]);
    }

    // clean up heap memory
    delete [] tArr;
    delete [] yArr;
    return 0;
}
