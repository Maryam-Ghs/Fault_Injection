/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <cmath>

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

    // ---- manual unrolling for a fixed size of 8 -----------------
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

    // element 4
    mom[4] = b1 * mom[4] + (1.0f - b1) * grad[4];
    vel[4] = b2 * vel[4] + (1.0f - b2) * grad[4] * grad[4];
    float m_hat4 = mom[4] / corr1;
    float v_hat4 = vel[4] / corr2;
    param[4] -= lr_corr * (m_hat4 / (std::sqrt(v_hat4) + eps)) + lr_wd * param[4];

    // element 5
    mom[5] = b1 * mom[5] + (1.0f - b1) * grad[5];
    vel[5] = b2 * vel[5] + (1.0f - b2) * grad[5] * grad[5];
    float m_hat5 = mom[5] / corr1;
    float v_hat5 = vel[5] / corr2;
    param[5] -= lr_corr * (m_hat5 / (std::sqrt(v_hat5) + eps)) + lr_wd * param[5];

    // element 6
    mom[6] = b1 * mom[6] + (1.0f - b1) * grad[6];
    vel[6] = b2 * vel[6] + (1.0f - b2) * grad[6] * grad[6];
    float m_hat6 = mom[6] / corr1;
    float v_hat6 = vel[6] / corr2;
    param[6] -= lr_corr * (m_hat6 / (std::sqrt(v_hat6) + eps)) + lr_wd * param[6];

    // element 7
    mom[7] = b1 * mom[7] + (1.0f - b1) * grad[7];
    vel[7] = b2 * vel[7] + (1.0f - b2) * grad[7] * grad[7];
    float m_hat7 = mom[7] / corr1;
    float v_hat7 = vel[7] / corr2;
    param[7] -= lr_corr * (m_hat7 / (std::sqrt(v_hat7) + eps)) + lr_wd * param[7];
    // -------------------------------------------------------------
}

int main()
{
    // ---- deterministic pseudo‑random data (stack allocation) ----
    float wgt[8] = { 0.123f, -0.456f, 0.789f, -0.321f,
                     0.654f, -0.987f, 0.111f, -0.222f };
    float gradArr[8] = { 0.005f, -0.007f, 0.009f, -0.011f,
                         0.013f, -0.015f, 0.017f, -0.019f };
    float mom[8] = {0.0f};
    float vel[8] = {0.0f};

    // hyper‑parameters (deterministic but varied)
    float lrRate = 0.0005f;
    float b1 = 0.85f;
    float b2 = 0.995f;
    float eps = 1e-7f;
    float wd = 0.005f;

    // simulate a few optimizer steps
    int maxIter = 5;
    for (int iter = 1; iter <= maxIter; ++iter)
    {
        stepAdamW(wgt, gradArr, mom, vel, iter, lrRate, b1, b2, eps, wd);
    }

    // ---- output final parameters ---------------------------------
    std::cout << "Final parameters after " << maxIter << " AdamW steps:\n";
    for (int i = 0; i < 8; ++i)
        std::cout << "w[" << i << "] = " << wgt[i] << '\n';

    return 0;
}
