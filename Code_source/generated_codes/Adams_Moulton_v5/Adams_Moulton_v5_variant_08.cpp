#include <cstdio>

/* LLM input variant 8: sparse-skewed */

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
    // deterministic sparse-skewed test vector
    int nSteps   = 20;          // number of integration steps
    // step sizes: many zeros, occasional non‑zero jumps
    float *stepSizes = new float[nSteps];
    for (int i = 0; i < nSteps; ++i) {
        if (i % 7 == 0) stepSizes[i] = 0.1f;          // occasional small step
        else if (i % 7 == 3) stepSizes[i] = 0.4f;    // occasional larger step
        else stepSizes[i] = 0.0f;                     // sparse zero steps
    }

    // heap‑allocated storage
    float *tArr = new float[nSteps + 1];
    float *yArr = new float[nSteps + 1];

    // initial condition y(0) = 1
    tArr[0] = 0.0f;
    yArr[0] = 1.0f;

    // first step – explicit Euler (provides the second seed)
    float h0 = stepSizes[0];
    tArr[1] = tArr[0] + h0;
    yArr[1] = yArr[0] + h0 * deriv(tArr[0], yArr[0]);

    // second step – predictor then corrector (manual unrolling)
    float h1 = stepSizes[1];
    tArr[2] = tArr[1] + h1;
    float yPred2 = predictAB2(tArr[1], yArr[1], tArr[0], yArr[0], h1);
    yArr[2] = correctAM2(tArr[1], yArr[1], tArr[2], yPred2, h1);

    // remaining steps – while loop with manual unrolled start
    int idx = 3;
    while (idx <= nSteps) {
        float h = stepSizes[idx - 1];
        tArr[idx] = tArr[idx - 1] + h;

        // predictor (AB2)
        float yPred = predictAB2(tArr[idx - 1], yArr[idx - 1],
                                 tArr[idx - 2], yArr[idx - 2],
                                 h);

        // corrector (AM2)
        yArr[idx] = correctAM2(tArr[idx - 1], yArr[idx - 1],
                               tArr[idx], yPred, h);

        idx = idx + 1;   // manual increment (unrolled style)
    }

    // output the solution
    for (int k = 0; k <= nSteps; ++k) {
        printf("%.2f %.6f\n", tArr[k], yArr[k]);
    }

    // clean up heap memory
    delete [] stepSizes;
    delete [] tArr;
    delete [] yArr;
    return 0;
}
