// AdaGrad Optimizer – version 4
/* LLM input variant 3: zeros-and-ones */
#include <cstdio>
#include <cmath>

int main() {
    // hyper‑parameters
    float learn = 1.0f;          // emphasize identity scaling
    float epsilon = 1e-6f;
    int steps = 12;
    int dataCnt = 4;

    // heap‑allocated tiny dataset with zeros and ones
    float* xv = new float[dataCnt];
    float* yv = new float[dataCnt];
    xv[0] = 0.0f; yv[0] = 0.0f;
    xv[1] = 1.0f; yv[1] = 1.0f;
    xv[2] = 0.0f; yv[2] = 0.0f;
    xv[3] = 1.0f; yv[3] = 1.0f;

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
        float rootW = cache[0];
        rootW = sqrtf(rootW);
        float denW = rootW + epsilon;
        float stepW = learn / denW;
        float deltaW = stepW * gradW;
        theta[0] = theta[0] - deltaW;

        // ----- update b -----
        float sqB = gradB * gradB;
        cache[1] = cache[1] + sqB;
        float rootB = cache[1];
        rootB = sqrtf(rootB);
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
