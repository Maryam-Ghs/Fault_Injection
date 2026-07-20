/* LLM input variant 9: medium-deterministic-random */
// AdaGrad Optimizer – version 4 (input variant 9)
#include <cstdio>
#include <cmath>

int main() {
    // hyper‑parameters
    float learn = 0.05f;
    float epsilon = 1e-6f;
    int steps = 20;
    int dataCnt = 10;

    // heap‑allocated deterministic pseudo‑random dataset
    float* xv = new float[dataCnt];
    float* yv = new float[dataCnt];
    xv[0] = 0.5f;   yv[0] = 1.6f;
    xv[1] = 1.7f;   yv[1] = 4.0f;
    xv[2] = -2.3f;  yv[2] = -4.1f;
    xv[3] = 3.2f;   yv[3] = 6.9f;
    xv[4] = -1.1f;  yv[4] = -1.8f;
    xv[5] = 4.0f;   yv[5] = 8.2f;
    xv[6] = -0.6f;  yv[6] = -0.7f;
    xv[7] = 2.5f;   yv[7] = 5.6f;
    xv[8] = -3.3f;  yv[8] = -6.2f;
    xv[9] = 5.1f;   yv[9] = 10.9f;

    // parameters: w (index 0) and b (index 1)
    float* theta = new float[2];
    theta[0] = 0.0f;   // w
    theta[1] = 0.0f;   // b

    // AdaGrad accumulators
    float* cache = new float[2];
    cache[0] = 0.0f;
    cache[1] = 0.0f;

    int iter = 0;
    while (iter < steps) {
        // accumulate gradients over the whole batch
        float gradW = 0.0f;
        float gradB = 0.0f;
        int idx = 0;
        while (idx < dataCnt) {
            float pred = theta[0] * xv[idx] + theta[1];
            float diff = pred - yv[idx];
            float gW = diff * xv[idx] * 2.0f;
            float gB = diff * 2.0f;
            gradW = gradW + gW;
            gradB = gradB + gB;
            idx = idx + 1;
        }

        // ----- update w -----
        float sqW = gradW * gradW;
        cache[0] = cache[0] + sqW;
        float rootW = sqrtf(cache[0]);
        float denW = rootW + epsilon;
        float stepW = learn / denW;
        float deltaW = stepW * gradW;
        theta[0] = theta[0] - deltaW;

        // ----- update b -----
        float sqB = gradB * gradB;
        cache[1] = cache[1] + sqB;
        float rootB = sqrtf(cache[1]);
        float denB = rootB + epsilon;
        float stepB = learn / denB;
        float deltaB = stepB * gradB;
        theta[1] = theta[1] - deltaB;

        iter = iter + 1;
    }

    // output final parameters
    printf("Final weight (w): %f\n", theta[0]);
    printf("Final bias   (b): %f\n", theta[1]);

    // clean‑up
    delete[] xv;
    delete[] yv;
    delete[] theta;
    delete[] cache;
    return 0;
}
