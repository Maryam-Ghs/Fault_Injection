#include <cstdio>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // hyper‑parameters (no const, only float)
    float eta   = 0.01f;      // learning rate
    float beta1 = 0.9f;       // momentum decay
    float beta2 = 0.999f;     // second‑moment decay
    float eps   = 1e-8f;      // stability term

    // deterministic gradient sequence: 5 steps, 2 parameters (duplicate‑heavy)
    float grad[5][2] = {
        { 0.10f, -0.20f },
        { 0.10f, -0.20f },   // duplicate of step 0
        { 0.05f,  0.05f },
        { 0.05f,  0.05f },   // duplicate of step 2
        { 0.00f,  0.00f }    // repeated zero gradient
    };

    // stack‑allocated optimizer state
    float w[2]      = { 0.5f, -0.5f };   // parameters
    float m[2]      = { 0.0f,  0.0f };   // first moment
    float v[2]      = { 0.0f,  0.0f };   // second moment
    float v_hat[2]  = { 0.0f,  0.0f };   // max of v

    int t = 0;
    while (t < 5) {
        // ---- load current gradient ------------------------------------------------
        float g0 = grad[t][0];
        float g1 = grad[t][1];

        // ---- first‑moment update --------------------------------------------------
        float m0_tmp = beta1 * m[0] + (1.0f - beta1) * g0;
        float m1_tmp = beta1 * m[1] + (1.0f - beta1) * g1;

        // ---- second‑moment update -------------------------------------------------
        float g0_sq = g0 * g0;
        float g1_sq = g1 * g1;
        float v0_tmp = beta2 * v[0] + (1.0f - beta2) * g0_sq;
        float v1_tmp = beta2 * v[1] + (1.0f - beta2) * g1_sq;

        // ---- max‑second‑moment (branch‑free) --------------------------------------
        float vhat0_tmp = v_hat[0] + fmaxf(v0_tmp - v_hat[0], 0.0f);
        float vhat1_tmp = v_hat[1] + fmaxf(v1_tmp - v_hat[1], 0.0f);

        // ---- compute denominator ---------------------------------------------------
        float den0 = sqrtf(vhat0_tmp) + eps;
        float den1 = sqrtf(vhat1_tmp) + eps;

        // ---- step size ------------------------------------------------------------
        float step0 = eta / den0;
        float step1 = eta / den1;

        // ---- parameter update ------------------------------------------------------
        float w0_new = w[0] - step0 * m0_tmp;
        float w1_new = w[1] - step1 * m1_tmp;

        // ---- commit new state ------------------------------------------------------
        m[0]      = m0_tmp;    m[1]      = m1_tmp;
        v[0]      = v0_tmp;    v[1]      = v1_tmp;
        v_hat[0]  = vhat0_tmp; v_hat[1]  = vhat1_tmp;
        w[0]      = w0_new;    w[1]      = w1_new;

        // ---- output ---------------------------------------------------------------
        printf("step %d: w0 = %.6f   w1 = %.6f\n", t + 1, w[0], w[1]);

        t = t + 1;   // advance loop counter without a for‑loop
    }

    return 0;
}
