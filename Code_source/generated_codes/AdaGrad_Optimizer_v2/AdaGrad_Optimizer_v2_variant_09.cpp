#include <cstdio>
#include <cmath>

#define DIM 10
#define STEPS 20

/* LLM input variant 9: medium-deterministic-random */

// deterministic pseudo‑random initializer for weights (range approx. [-0.5, 0.5])
void init_w(float *w) {
    unsigned int state = 123456789u; // fixed seed
    int i = 0;
    while (i < DIM) {
        // simple linear congruential generator
        state = state * 1103515245u + 12345u;
        // take high 31 bits, scale to [-0.5, 0.5]
        w[i] = ((float)(state & 0x7fffffff) / 2147483648.0f) - 0.5f;
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

// generate a deterministic, diverse gradient pattern (includes zeros and large magnitude values)
void gen_g(const float *w, float *g, int step) {
    int i = 0;
    while (i < DIM) {
        // combine index and step to obtain a pseudo‑random integer
        int tmp = (i * 7 + step * 3) % 13; // values 0..12
        float base;
        if (tmp == 0) {
            base = 0.0f;                     // explicit zero case
        } else if (tmp % 2 == 0) {
            base = 1e4f * (float)tmp;        // positive large values
        } else {
            base = -1e4f * (float)tmp;       // negative large values
        }
        // small offset to avoid exact repeats
        float offset = (float)(step % 5) * 0.0005f;
        g[i] = base + offset;
        i = i + 1;
    }
}

// one AdaGrad update (branch‑minimized, math split into temporaries)
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
    float base_lr = 0.05f;
    float epsilon = 1e-8f;

    // stack‑allocated vectors
    float weights[DIM];
    float accum[DIM];
    float grads[DIM];

    // initialization
    init_w(weights);
    init_a(accum);

    // main optimisation loop (while)
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
