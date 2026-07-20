#include <cstdio>
#include <cmath>

/* LLM input variant 2: small-diverse */

#define DIM 5
#define STEPS 12

// initialize weights to a different set of small diverse values
void init_w(float *w) {
    float seed[DIM] = {0.2f, -0.3f, 0.0f, 0.8f, -1.0f};
    int i = 0;
    while (i < DIM) {
        w[i] = seed[i];
        i = i + 1;
    }
}

// zero the accumulator (unchanged)
void init_a(float *a) {
    int i = 0;
    while (i < DIM) {
        a[i] = 0.0f;
        i = i + 1;
    }
}

// produce a deterministic gradient pattern with varied magnitudes
void gen_g(const float *w, float *g, int step) {
    int i = 0;
    while (i < DIM) {
        // pattern: 2.5, -2.5, 0, 1e2, -1e2 (repeated each step with small offset)
        float base = (i == 0) ? 2.5f :
                     (i == 1) ? -2.5f :
                     (i == 2) ? 0.0f :
                     (i == 3) ? 1e2f :
                     -1e2f;
        // add a tiny term depending on step to avoid exact repetition
        float offset = (float)(step % 5) * 0.0005f;
        g[i] = base + offset;
        i = i + 1;
    }
}

// one AdaGrad update (unchanged)
void adagrad_step(float *w, float *a, const float *g, float lr, float eps) {
    int i = 0;
    while (i < DIM) {
        float grad = g[i];
        float sq = grad * grad;
        float acc = a[i] + sq;
        float sqrt_acc = sqrtf(acc + eps);
        float adj_lr = lr / sqrt_acc;
        float delta = adj_lr * grad;
        w[i] = w[i] - delta;
        a[i] = acc;
        i = i + 1;
    }
}

// print current weights (unchanged)
void show_w(const float *w, int iter) {
    printf("Iter %2d :", iter);
    int i = 0;
    while (i < DIM) {
        printf(" % .6f", w[i]);
        i = i + 1;
    }
    printf("\n");
}

int main() {
    // hyper‑parameters (modified for this variant)
    float base_lr = 0.05f;
    float epsilon = 1e-6f;

    // stack‑allocated vectors
    float weights[DIM];
    float accum[DIM];
    float grads[DIM];

    // initialization
    init_w(weights);
    init_a(accum);

    // main optimisation loop
    int step = 0;
    while (step < STEPS) {
        gen_g(weights, grads, step);
        adagrad_step(weights, accum, grads, base_lr, epsilon);
        show_w(weights, step + 1);
        step = step + 1;
    }

    // final result
    printf("Final weights:");
    int i = 0;
    while (i < DIM) {
        printf(" % .6f", weights[i]);
        i = i + 1;
    }
    printf("\n");
    return 0;
}
