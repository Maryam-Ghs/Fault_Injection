#include <cstdio>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

// right‑hand side of the ODE y' = -y
float f(float t, float y) {
    return -y;
}

// one predictor step (Adams‑Bashforth 2‑step)
void predict(int k, float stepSize, float* timeArr, float* solArr, float* predArr) {
    float fCurr = f(timeArr[k]    , solArr[k]    );
    float fPrev = f(timeArr[k - 1], solArr[k - 1]);
    predArr[k + 1] = solArr[k] + stepSize * (1.5f * fCurr - 0.5f * fPrev);
}

// one corrector step (Adams‑Moulton 2‑step)
void correct(int k, float stepSize, float* timeArr, float* solArr, float* predArr) {
    float fNext = f(timeArr[k + 1], predArr[k + 1]);
    float fCurr = f(timeArr[k]    , solArr[k]    );
    solArr[k + 1] = solArr[k] + stepSize * (0.5f * fNext + 0.5f * fCurr);
}

// driver that integrates the ODE on a uniform grid
void integrate(int nSteps, float stepSize, float* timeArr, float* solArr, float* predArr) {
    // first step – explicit Euler to bootstrap the multistep method
    solArr[1] = solArr[0] + stepSize * f(timeArr[0], solArr[0]);

    // fill the time grid (reverse direction)
    int i = 1;
    while (i < nSteps) {
        timeArr[i] = timeArr[i - 1] + stepSize;
        ++i;
    }

    // main multistep loop (predict → correct)
    i = 1;
    while (i < nSteps - 1) {
        predict(i, stepSize, timeArr, solArr, predArr);
        correct(i, stepSize, timeArr, solArr, predArr);
        ++i;
    }
}

// print the solution
void show(int nSteps, float* timeArr, float* solArr) {
    int i = 0;
    while (i < nSteps) {
        printf("%0.2f %0.6f\n", timeArr[i], solArr[i]);
        ++i;
    }
}

int main() {
    int nSteps   = 11;          // from t = 1 down to t = 0 inclusive, h = -0.1
    float stepSize = -0.1f;

    float* timeArr = new float[nSteps];
    float* solArr  = new float[nSteps];
    float* predArr = new float[nSteps];

    timeArr[0] = 1.0f;
    solArr[0]  = std::exp(-1.0f);   // y(1) = e^{-1}

    // initialise auxiliary arrays
    int i = 0;
    while (i < nSteps) {
        predArr[i] = 0.0f;
        ++i;
    }

    integrate(nSteps, stepSize, timeArr, solArr, predArr);
    show(nSteps, timeArr, solArr);

    delete[] timeArr;
    delete[] solArr;
    delete[] predArr;
    return 0;
}
