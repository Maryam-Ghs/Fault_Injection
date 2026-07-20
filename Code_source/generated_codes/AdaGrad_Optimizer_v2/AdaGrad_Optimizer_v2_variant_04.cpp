#include <cstdio>
#include <cmath>

#define DIM 5
#define STEPS 10

/* LLM input variant 4: signed-extremes */

// initialize weights to a new mix of negative, zero, and positive values
void init_w(float *w) {
    float seed[DIM] = { -2.0f, 0.0f, 3.5f, -4.2f, 0.0f };
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

// produce a deterministic gradient pattern with different magnitudes
void gen_g(const float *w, float *g, int step) {
    int i = 0;
    while (i < DIM) {
        // pattern: -1e4, 0, 2e4, -5e5, 0 (repeated each step with a tiny offset)
        float base = (i == 0) ? -1e4f :
                     (i == 1) ? 0.0f :
                     (i == 2) ? 2e4f :
                     (i == 3) ? -5e5f :
                     0.0f;
        // small offset depending on step
        float offset = (float)(step % 4) * 0.002f;
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
    // hyper‑parameters (adjusted)
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
