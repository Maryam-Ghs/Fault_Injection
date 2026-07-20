/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <cmath>

// allocate all needed vectors on the heap
void allocate_vectors(int n,
                      float** vec_param,
                      float** vec_accGrad,
                      float** vec_accUpd,
                      float** vec_grad,
                      float** vec_delta)
{
    *vec_param   = new float[n];
    *vec_accGrad = new float[n];
    *vec_accUpd  = new float[n];
    *vec_grad    = new float[n];
    *vec_delta   = new float[n];
}

// initialise parameters and accumulators
void initialise(int n, float* param, float* accGrad, float* accUpd)
{
    int i = 0;
    while (i < n) {
        // alternate zeros and ones
        param[i] = (i % 2 == 0) ? 0.0f : 1.0f;
        accGrad[i] = 0.0f;
        accUpd[i] = 0.0f;
        i = i + 1;
    }
}

// compute a synthetic gradient (here d/dx (x^2) = 2*x)
void compute_gradient(int n, float* param, float* grad)
{
    int i = 0;
    while (i < n) {
        float two = 2.0f;
        grad[i] = two * param[i];
        i = i + 1;
    }
}

// one Adadelta update step with expanded arithmetic
void adadelta_step(int n,
                   float* param,
                   float* accGrad,
                   float* accUpd,
                   float* grad,
                   float* delta,
                   float rho,
                   float eps)
{
    int i = 0;
    while (i < n) {
        // 1) accumulate squared gradients
        float g = grad[i];
        float g_sq = g * g;
        float one_minus_rho = 1.0f - rho;
        float term1 = rho * accGrad[i];
        float term2 = one_minus_rho * g_sq;
        accGrad[i] = term1 + term2;

        // 2) compute RMS of accumulated gradients
        float rms_g_sq = accGrad[i] + eps;
        float rms_g = sqrt(rms_g_sq);

        // 3) compute RMS of accumulated updates (previous step)
        float rms_u_sq = accUpd[i] + eps;
        float rms_u = sqrt(rms_u_sq);

        // 4) compute parameter update
        float ratio = rms_u / rms_g;
        delta[i] = - ratio * g;

        // 5) apply update
        param[i] = param[i] + delta[i];

        // 6) accumulate squared updates
        float d_sq = delta[i] * delta[i];
        float term3 = rho * accUpd[i];
        float term4 = one_minus_rho * d_sq;
        accUpd[i] = term3 + term4;

        i = i + 1;
    }
}

// clean up heap memory
void cleanup(float* param,
             float* accGrad,
             float* accUpd,
             float* grad,
             float* delta)
{
    delete[] param;
    delete[] accGrad;
    delete[] accUpd;
    delete[] grad;
    delete[] delta;
}

int main()
{
    // problem size
    int dim = 6;

    // hyper‑parameters
    float rho = 0.0f;      // emphasize zero weighting
    float eps = 1e-6f;
    int epochs = 1;        // minimal non‑empty run

    // allocate vectors
    float* param   = nullptr;
    float* accGrad = nullptr;
    float* accUpd  = nullptr;
    float* grad    = nullptr;
    float* delta   = nullptr;

    allocate_vectors(dim, &param, &accGrad, &accUpd, &grad, &delta);

    // initialise everything
    initialise(dim, param, accGrad, accUpd);

    // optimisation loop
    int e = 0;
    while (e < epochs) {
        compute_gradient(dim, param, grad);
        adadelta_step(dim, param, accGrad, accUpd, grad, delta, rho, eps);
        e = e + 1;
    }

    // output final parameters
    std::cout << "Final parameters after " << epochs << " epochs:\n";
    int i = 0;
    while (i < dim) {
        std::cout << "param[" << i << "] = " << param[i] << "\n";
        i = i + 1;
    }

    // free memory
    cleanup(param, accGrad, accUpd, grad, delta);
    return 0;
}
