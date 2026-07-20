#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// AdamW optimizer – version #7
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

    // ---- manual unrolling for a fixed size of 4 -----------------
    // element 0
    mom[0] = b1 * mom[0] + (1.0f - b1) * grad[0];
    vel[0] = b2 * vel[0] + (1.0f - b2) * grad[0] * grad[0];
    float m_hat0 = mom[0] / corr1;
    float v_hat0 = vel[0] / corr2;
    param[0] -= lr_corr * (m_hat0 / (std::sqrt(v_hat0) + eps)) + lr_wd * param[0];

    // element 1
    mom[1] = b1 * mom[1] + (1.0f - b1) * grad[1];
    vel[1] = b2 * vel[1] + (1.0f - b2) * grad[1] * grad[1];
    float m_hat1 = mom[1] / corr1;
    float v_hat1 = vel[1] / corr2;
    param[1] -= lr_corr * (m_hat1 / (std::sqrt(v_hat1) + eps)) + lr_wd * param[1];

    // element 2
    mom[2] = b1 * mom[2] + (1.0f - b1) * grad[2];
    vel[2] = b2 * vel[2] + (1.0f - b2) * grad[2] * grad[2];
    float m_hat2 = mom[2] / corr1;
    float v_hat2 = vel[2] / corr2;
    param[2] -= lr_corr * (m_hat2 / (std::sqrt(v_hat2) + eps)) + lr_wd * param[2];

    // element 3
    mom[3] = b1 * mom[3] + (1.0f - b1) * grad[3];
    vel[3] = b2 * vel[3] + (1.0f - b2) * grad[3] * grad[3];
    float m_hat3 = mom[3] / corr1;
    float v_hat3 = vel[3] / corr2;
    param[3] -= lr_corr * (m_hat3 / (std::sqrt(v_hat3) + eps)) + lr_wd * param[3];
    // -------------------------------------------------------------
}

int main()
{
    // ---- small predefined data (stack allocation) ----------------
    float wgt[4] = {0.5f, -0.3f, 0.8f, -1.2f};   // initial parameters
    float gradArr[4] = {0.02f, -0.01f, 0.03f, -0.04f}; // mock gradients
    float mom[4] = {0.0f, 0.0f, 0.0f, 0.0f};    // first moment buffers
    float vel[4] = {0.0f, 0.0f, 0.0f, 0.0f};    // second moment buffers

    // hyper‑parameters (chosen arbitrarily for demonstration)
    float lrRate = 0.001f;
    float b1 = 0.9f;
    float b2 = 0.999f;
    float eps = 1e-8f;
    float wd = 0.01f;

    // simulate a large number of optimizer steps (stress test)
    int maxIter = 1000000;  // one million iterations
    int iter = 1;
    while (iter <= maxIter)
    {
        stepAdamW(wgt, gradArr, mom, vel, iter, lrRate, b1, b2, eps, wd);
        ++iter;
    }

    // ---- output final parameters ---------------------------------
    std::cout << "Final parameters after " << maxIter << " AdamW steps:\n";
    for (int i = 0; i < 4; ++i)
        std::cout << "w[" << i << "] = " << wgt[i] << '\n';

    return 0;
}
