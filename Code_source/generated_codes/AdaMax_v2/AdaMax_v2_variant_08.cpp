#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 8: sparse-skewed */

// generate a sparse, clustered vector on the stack
void fill_vec(float *v, int n) {
    // first 1/8th of the entries are non‑zero with a wider range,
    // the rest are zero to create sparsity and clustering.
    int dense_limit = n / 8;
    for (int i = 0; i < n; ++i) {
        if (i < dense_limit) {
            // widened random range to add skew
            v[i] = ((float)std::rand() / RAND_MAX - 0.5f) * 10.0f; // roughly [-5,5]
        } else {
            v[i] = 0.0f;
        }
    }
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
    // deterministic seed for reproducibility
    std::srand(42U);

    // medium size, all on the stack
    int N = 64;
    float w[64];   // parameters
    float g[64];   // gradients
    float mt[64];  // first‑moment estimate
    float ut[64];  // infinity‑norm estimate

    // initialise everything with sparse, clustered data
    fill_vec(w, N);
    fill_vec(g, N);
    for (int i = 0; i < N; ++i) {
        mt[i] = 0.0f;
        ut[i] = 0.0f;
    }

    // AdaMax hyper‑parameters (float only)
    float lr   = 0.002f;
    float b1   = 0.9f;
    float b2   = 0.999f;
    float eps  = 1e-8f;

    // run a few optimisation steps – while loop gives a different loop shape
    int t = 1;
    int max_steps = 10;
    while (t <= max_steps) {
        // pretend new gradients appear each step (sparse, clustered)
        fill_vec(g, N);
        ada_max_step(w, g, mt, ut, N, lr, b1, b2, eps, t);
        ++t;
    }

    // output final parameters
    show_vec("Final parameters after AdaMax:", w, N);
    return 0;
}
