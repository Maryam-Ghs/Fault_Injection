#include <cstdio>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

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

    // fill the time grid
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

// simple deterministic pseudo‑random generator (linear congruential)
float pseudo_random(int seed) {
    const unsigned int a = 1664525u;
    const unsigned int c = 1013904223u;
    const unsigned int m = 0xFFFFFFFFu;
    unsigned int x = static_cast<unsigned int>(seed);
    x = (a * x + c) & m;
    return static_cast<float>(x) / static_cast<float>(m);
}

int main() {
    int nSteps   = 15;          // from t = 0 to approx 0.98, h = 0.07
    float stepSize = 0.07f;

    float* timeArr = new float[nSteps];
    float* solArr  = new float[nSteps];
    float* predArr = new float[nSteps];

    timeArr[0] = 0.0f;
    solArr[0]  = 0.93f;          // deterministic non‑trivial initial value

    // initialise auxiliary array with deterministic pseudo‑random values
    for (int i = 0; i < nSteps; ++i) {
        predArr[i] = pseudo_random(i * 12345);
    }

    integrate(nSteps, stepSize, timeArr, solArr, predArr);
    show(nSteps, timeArr, solArr);

    delete[] timeArr;
    delete[] solArr;
    delete[] predArr;
    return 0;
}
