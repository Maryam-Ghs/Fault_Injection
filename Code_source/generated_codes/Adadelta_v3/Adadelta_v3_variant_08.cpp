#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

class AdadeltaOpt {
public:
    // hyper‑parameters
    float decay;
    float eps;
    int dim;

    // running averages (stored on the stack via the class instance)
    float avg_sq_grad[16];
    float avg_sq_upd[16];

    // constructor – all members are plain floats/ints, no const, no double
    AdadeltaOpt(float d, float e, int n) {
        decay = d;
        eps   = e;
        dim   = n;
        // initialise averages to zero
        for (int i = 0; i < dim; ++i) {
            avg_sq_grad[i] = 0.0f;
            avg_sq_upd[i]  = 0.0f;
        }
    }

    // single optimisation step
    void step(float *par, float *grd) {
        // temporary storage for the update (Δx) to keep the formula readable
        float delta[16];

        // loop over parameters – no conditional branches inside
        for (int i = 0; i < dim; ++i) {
            // 1) update average of squared gradients:   m_t = rho*m_{t-1} + (1‑rho)*g^2
            float grad_sq = grd[i] * grd[i];
            float term_a  = decay * avg_sq_grad[i];
            float term_b  = (1.0f - decay) * grad_sq;
            avg_sq_grad[i] = term_a + term_b;

            // 2) compute RMS of past updates: sqrt(E[Δx^2]_{t‑1} + eps)
            float rms_upd = std::sqrt(avg_sq_upd[i] + eps);

            // 3) compute RMS of current gradients: sqrt(E[g^2]_t + eps)
            float rms_grad = std::sqrt(avg_sq_grad[i] + eps);

            // 4) compute raw update: - (rms_upd / rms_grad) * g
            float ratio = rms_upd / rms_grad;
            float raw_upd = - ratio * grd[i];

            // 5) store raw update
            delta[i] = raw_upd;

            // 6) update average of squared updates:   v_t = rho*v_{t-1} + (1‑rho)*Δx^2
            float upd_sq = raw_upd * raw_upd;
            float term_c = decay * avg_sq_upd[i];
            float term_d = (1.0f - decay) * upd_sq;
            avg_sq_upd[i] = term_c + term_d;

            // 7) apply update to parameter
            par[i] = par[i] + delta[i];
        }
    }
};

int main() {
    // deterministic test vector size – using full capacity but sparsely populated
    const int N = 16;
    float params[N] = {
        1.0f,    // dense region start
        0.0f,    // zero (sparse)
        0.0f,    // zero (sparse)
        0.0f,    // zero (sparse)
        0.0f,    // zero (sparse)
        0.0f,    // zero (sparse)
        0.0f,    // zero (sparse)
        0.0f,    // zero (sparse)
        -2.0f,   // second non‑zero far from start (clustered)
        0.0f,
        0.0f,
        3.5f,    // third non‑zero, larger magnitude (skewed)
        0.0f,
        0.0f,
        0.0f,
        0.0f
    };

    // deterministic gradient sequence – mostly zeros with few varied magnitudes
    float grads[N] = {
        0.0f,        // zero gradient
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.5f,        // moderate gradient far in array
        0.0f,
        0.0f,
        -0.001f,     // tiny gradient (skewed small)
        0.0f,
        0.0f,
        0.0f,
        0.0f
    };

    // hyper‑parameters (rho and epsilon) – chosen as typical defaults
    float rho = 0.95f;
    float epsilon = 1e-8f; // tighter epsilon to handle tiny gradients

    // create optimizer instance
    AdadeltaOpt optimizer(rho, epsilon, N);

    // run a few deterministic steps
    const int STEPS = 3;
    int step = 0;
    while (step < STEPS) {
        optimizer.step(params, grads);
        ++step;
    }

    // output final parameters
    std::cout << "Final parameters after " << STEPS << " Adadelta steps:\n";
    for (int i = 0; i < N; ++i) {
        std::cout << "p[" << i << "] = " << params[i] << '\n';
    }
    return 0;
}
