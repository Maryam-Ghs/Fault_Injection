#include <iostream>
#include <cmath>

/* LLM input variant 1: minimal-boundary */

// ------------------------------------------------------------
// AdamW optimizer – version #7 (minimal boundary variant)
// ------------------------------------------------------------
void stepAdamW(
    float *param,      // model weights
    float *grad,       // gradients
    float *mom,        // first moment
    float *vel,        // second moment
    int   iter,        // time step (1‑based)
    float lrRate,      // learning rate
    float b1,          // beta1
    float b2,          // beta2
    float eps,         // epsilon
    float wd)          // weight decay
{
    // bias‑correction factors (pre‑computed for this step)
    float pow_b1 = std::pow(b1, static_cast<float>(iter));
    float pow_b2 = std::pow(b2, static_cast<float>(iter));
    float corr1  = 1.0f - pow_b1;
    float corr2  = 1.0f - pow_b2;

    // fused learning‑rate factor (lr / sqrt(corr2) + weight decay term)
    float lr_corr = lrRate / std::sqrt(corr2);
    float lr_wd   = lrRate * wd;

    // ---- manual unrolling for a fixed size of 1 -----------------
    // element 0
    mom[0] = b1 * mom[0] + (1.0f - b1) * grad[0];
    vel[0] = b2 * vel[0] + (1.0f - b2) * grad[0] * grad[0];
    float m_hat0 = mom[0] / corr1;
    float v_hat0 = vel[0] / corr2;
    param[0] -= lr_corr * (m_hat0 / (std::sqrt(v_hat0) + eps)) + lr_wd * param[0];
    // -------------------------------------------------------------
}

int main()
{
    // ---- minimal predefined data (single element) ----------------
    float wgt[1] = {0.5f};                     // initial parameter
    float gradArr[1] = {0.02f};                // mock gradient
    float mom[1] = {0.0f};                     // first moment buffer
    float vel[1] = {0.0f};                     // second moment buffer

    // hyper‑parameters (boundary‑valid minimal values)
    float lrRate = 0.001f;
    float b1 = 0.9f;
    float b2 = 0.999f;
    float eps = 1e-8f;
    float wd = 0.01f;

    // simulate a single optimizer step
    int maxIter = 1;
    int iter = 1;
    while (iter <= maxIter)
    {
        stepAdamW(wgt, gradArr, mom, vel, iter, lrRate, b1, b2, eps, wd);
        ++iter;
    }

    // ---- output final parameter ---------------------------------
    std::cout << "Final parameter after " << maxIter << " AdamW step:\n";
    std::cout << "w[0] = " << wgt[0] << '\n';

    return 0;
}
