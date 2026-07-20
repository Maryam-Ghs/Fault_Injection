#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

// version #5

// ------------------------------------------------------------
// Helper: compute simple squared error gradient for a linear model
// ------------------------------------------------------------
void compute_gradients(
    const std::vector<float>& data0,
    const std::vector<float>& data1,
    float target,
    const std::vector<float>& param,
    std::vector<float>& grad)
{
    // prediction = param[0] * x0 + param[1] * x1
    float pred = param[0] * data0[0] + param[1] * data1[0];
    // error = pred - y
    float err = pred - target;

    // gradient = 2 * error * x
    grad[0] = 2.0f * err * data0[0];
    grad[1] = 2.0f * err * data1[0];
}

// ------------------------------------------------------------
// Helper: one AdaDelta update step (manual unrolling for 2 params)
// ------------------------------------------------------------
void adadelta_update(
    std::vector<float>& param,
    std::vector<float>& grad_acc,
    std::vector<float>& upd_acc,
    const std::vector<float>& grad,
    float rho,
    float eps)
{
    // ----- param 0 -----
    float g0       = grad[0];
    float g0_sq    = g0 * g0;
    float ga0_old  = grad_acc[0];
    float ga0_new  = rho * ga0_old + (1.0f - rho) * g0_sq;
    float upd0_num = upd_acc[0] + eps;
    float upd0_den = ga0_new + eps;
    float rms_upd0 = std::sqrt(upd0_num);
    float rms_g0   = std::sqrt(upd0_den);
    float delta0   = - (rms_upd0 / rms_g0) * g0;
    float delta0_sq = delta0 * delta0;
    float up0_old  = upd_acc[0];
    float up0_new  = rho * up0_old + (1.0f - rho) * delta0_sq;

    param[0] += delta0;
    grad_acc[0] = ga0_new;
    upd_acc[0]   = up0_new;

    // ----- param 1 -----
    float g1       = grad[1];
    float g1_sq    = g1 * g1;
    float ga1_old  = grad_acc[1];
    float ga1_new  = rho * ga1_old + (1.0f - rho) * g1_sq;
    float upd1_num = upd_acc[1] + eps;
    float upd1_den = ga1_new + eps;
    float rms_upd1 = std::sqrt(upd1_num);
    float rms_g1   = std::sqrt(upd1_den);
    float delta1   = - (rms_upd1 / rms_g1) * g1;
    float delta1_sq = delta1 * delta1;
    float up1_old  = upd_acc[1];
    float up1_new  = rho * up1_old + (1.0f - rho) * delta1_sq;

    param[1] += delta1;
    grad_acc[1] = ga1_new;
    upd_acc[1]   = up1_new;
}

// ------------------------------------------------------------
// Main driver (deterministic large‑scale test vectors, no I/O)
// ------------------------------------------------------------
int main()
{
    // deterministic large dataset: many samples, two features each
    const int sample_count = 5000;                     // safe yet sizable
    std::vector<float> feat0(sample_count);
    std::vector<float> feat1(sample_count);
    std::vector<float> label(sample_count);

    for (int i = 0; i < sample_count; ++i) {
        feat0[i] = 1.0f + 0.001f * i;   // slowly increasing x0
        feat1[i] = 2.0f + 0.001f * i;   // slowly increasing x1
        label[i] = 5.0f + 0.001f * i;   // target y
    }

    // model parameters (initialized to zero)
    std::vector<float> param = {0.0f, 0.0f};

    // AdaDelta state
    std::vector<float> grad_acc = {0.0f, 0.0f};
    std::vector<float> upd_acc  = {0.0f, 0.0f};

    // hyper‑parameters
    float rho = 0.95f;
    float eps = 1e-6f;

    // number of epochs
    int epoch = 5;

    // gradient buffer (size 2)
    std::vector<float> grad = {0.0f, 0.0f};

    // training loop
    for (int e = 0; e < epoch; ++e)
    {
        // accumulate gradients over all samples
        grad[0] = 0.0f;
        grad[1] = 0.0f;

        for (int i = 0; i < sample_count; ++i)
        {
            std::vector<float> tmp_grad(2);
            compute_gradients({feat0[i]}, {feat1[i]}, label[i], param, tmp_grad);
            grad[0] += tmp_grad[0];
            grad[1] += tmp_grad[1];
        }

        // average gradient
        float inv_n = 1.0f / static_cast<float>(sample_count);
        grad[0] *= inv_n;
        grad[1] *= inv_n;

        // one AdaDelta step
        adadelta_update(param, grad_acc, upd_acc, grad, rho, eps);
    }

    // print final parameters
    std::cout << "Final parameters after " << epoch << " epochs:\n";
    std::cout << "param[0] = " << param[0] << "\n";
    std::cout << "param[1] = " << param[1] << "\n";

    return 0;
}
