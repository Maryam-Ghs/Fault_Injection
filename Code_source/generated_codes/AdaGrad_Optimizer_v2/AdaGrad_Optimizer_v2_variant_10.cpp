#include <cstdio>
#include <cmath>

#define DIM 1000
#define STEPS 1000

/* LLM input variant 10: large-safe-stress */

// initialize weights to small deterministic values (repeat pattern from original)
void init_w(float *w) {
    float baseVals[5] = {0.0f, -0.1f, 0.5f, -0.5f, 1.0f};
    int i = 0;
    while (i < DIM) {
        w[i] = baseVals[i % 5];
        i = i + 1;
    }
}

// zero the accumulator (edge‑case: start from zero)
void init_a(float *a) {
    int i = 0;
    while (i < DIM) {
        a[i] = 0.0f;
        i = i + 1;
    }
}

// produce a deterministic gradient pattern (contains zeros and huge values)
void gen_g(const float *w, float *g, int step) {
    int i = 0;
    while (i < DIM) {
        // pattern repeats every 5 dimensions: 0, 1e3, -1e3, 0, 1e5
        float base = (i % 5 == 0) ? 0.0f :
                     (i % 5 == 1) ? 1e3f :
                     (i % 5 == 2) ? -1e3f :
                     (i % 5 == 3) ? 0.0f :
                     1e5f;
        // add a small term depending on step to avoid pure repetition
        float offset = (float)(step % 3) * 0.001f;
        g[i] = base + offset;
        i = i + 1;
    }
}

// one AdaGrad update (branch‑minimized, math split into temporaries)
void adagrad_step(float *w, float *a, const float *g, float lr, float eps) {
    int i = 0;
    while (i < DIM) {
        float grad = g[i];
        float sq = grad * grad;                 // temporary: square
        float acc = a[i] + sq;                  // accumulator update
        float sqrt_acc = sqrtf(acc + eps);      // sqrt with epsilon
        float adj_lr = lr / sqrt_acc;           // adjusted learning rate
        float delta = adj_lr * grad;            // step size
        w[i] = w[i] - delta;                    // weight update
        a[i] = acc;                             // store new accumulator
        i = i + 1;
    }
}

// print current weights (edge‑case: show sign and magnitude)
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
    // hyper‑parameters (float only)
    float base_lr = 0.1f;
    float epsilon = 1e-8f;

    // stack‑allocated vectors
    float weights[DIM];
    float accum[DIM];
    float grads[DIM];

    // initialization
    init_w(weights);
    init_a(accum);

    // main optimisation loop (changed loop structure: while)
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
