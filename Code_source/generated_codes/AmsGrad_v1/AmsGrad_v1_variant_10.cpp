#include <cstdio>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

int main() {
    // hyper‑parameters (no const, only float)
    float eta   = 0.01f;      // learning rate
    float beta1 = 0.9f;       // momentum decay
    float beta2 = 0.999f;     // second‑moment decay
    float eps   = 1e-8f;      // stability term

    // enlarged deterministic gradient sequence
    const int STEPS   = 1000;   // number of optimization steps
    const int PARAMS  = 10;     // number of parameters per step

    // stack‑allocated optimizer state for PARAMS dimensions
    float w[PARAMS]      = { 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    float m[PARAMS]      = { 0.0f };
    float v[PARAMS]      = { 0.0f };
    float v_hat[PARAMS]  = { 0.0f };

    // temporary storage for per‑step calculations
    float g[PARAMS];
    float m_tmp[PARAMS];
    float v_tmp[PARAMS];
    float vhat_tmp[PARAMS];
    float den[PARAMS];
    float step[PARAMS];

    for (int t = 0; t < STEPS; ++t) {
        // ---- generate deterministic gradient for this step --------------------
        for (int i = 0; i < PARAMS; ++i) {
            // simple sinusoidal pattern scaled to keep values modest
            g[i] = 0.1f * sinf(static_cast<float>(t + i));
        }

        // ---- first‑moment update ------------------------------------------------
        for (int i = 0; i < PARAMS; ++i) {
            m_tmp[i] = beta1 * m[i] + (1.0f - beta1) * g[i];
        }

        // ---- second‑moment update ------------------------------------------------
        for (int i = 0; i < PARAMS; ++i) {
            float g_sq = g[i] * g[i];
            v_tmp[i] = beta2 * v[i] + (1.0f - beta2) * g_sq;
        }

        // ---- max‑second‑moment (branch‑free) --------------------------------------
        for (int i = 0; i < PARAMS; ++i) {
            vhat_tmp[i] = v_hat[i] + fmaxf(v_tmp[i] - v_hat[i], 0.0f);
        }

        // ---- compute denominator -------------------------------------------------
        for (int i = 0; i < PARAMS; ++i) {
            den[i] = sqrtf(vhat_tmp[i]) + eps;
        }

        // ---- step size ------------------------------------------------------------
        for (int i = 0; i < PARAMS; ++i) {
            step[i] = eta / den[i];
        }

        // ---- parameter update ------------------------------------------------------
        for (int i = 0; i < PARAMS; ++i) {
            w[i] = w[i] - step[i] * m_tmp[i];
        }

        // ---- commit new state ------------------------------------------------------
        for (int i = 0; i < PARAMS; ++i) {
            m[i]      = m_tmp[i];
            v[i]      = v_tmp[i];
            v_hat[i]  = vhat_tmp[i];
        }

        // ---- output (show first two parameters for brevity) ----------------------
        printf("step %d: w0 = %.6f   w1 = %.6f\n", t + 1, w[0], w[1]);
    }

    return 0;
}
