#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

// generate a larger deterministic vector on the stack
void fill_vec(float *v, int n) {
    for (int i = 0; i < n; ++i)
        v[i] = (float)i / n - 0.5f;   // deterministic values in [-0.5,0.5)
}

// one AdaMax update (helper – no branches)
void ada_max_step(float *w, float *g, float *mt, float *ut,
                  int n, float lr, float b1, float b2,
                  float eps, int step) {
    float b1_pow = powf(b1, (float)step);
    float adj    = lr / (1.0f - b1_pow);               // learning‑rate correction

    for (int i = 0; i < n; ++i) {
        mt[i] = b1 * mt[i] + (1.0f - b1) * g[i];               // first moment
        ut[i] = fmaxf(b2 * ut[i], fabsf(g[i]));                // infinity norm
        w[i]  -= adj * (mt[i] / (ut[i] + eps));                // parameter update
    }
}

// print a vector (stack array)
void show_vec(const char *msg, float *v, int n) {
    std::printf("%s\n", msg);
    for (int i = 0; i < n; ++i)
        std::printf("% .5f ", v[i]);
    std::printf("\n");
}

int main() {
    std::srand((unsigned)std::time(0));

    // larger size, still on the stack (≈16 KB total)
    const int N = 1024;
    float w[N];   // parameters
    float g[N];   // gradients
    float mt[N];  // first‑moment estimate
    float ut[N];  // infinity‑norm estimate

    // initialise everything with deterministic data
    fill_vec(w, N);
    fill_vec(g, N);
    for (int i = 0; i < N; ++i) {
        mt[i] = 0.0f;
        ut[i] = 0.0f;
    }

    // AdaMax hyper‑parameters (float only)
    const float lr   = 0.002f;
    const float b1   = 0.9f;
    const float b2   = 0.999f;
    const float eps  = 1e-8f;

    // run a moderate number of optimisation steps
    int t = 1;
    const int max_steps = 20;
    while (t <= max_steps) {
        // deterministic new gradients each step
        fill_vec(g, N);
        ada_max_step(w, g, mt, ut, N, lr, b1, b2, eps, t);
        ++t;
    }

    // output final parameters
    show_vec("Final parameters after AdaMax:", w, N);
    return 0;
}
