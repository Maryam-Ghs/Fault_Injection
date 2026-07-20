#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

// Adadelta optimizer version #7
// --------------------------------------------------
// Helper: fill weight vector with a sparse start pattern
void seed_weights(float wgt[], int dim) {
    int idx = 0;
    while (idx < dim) {
        // Create sparsity: only every 5th element gets a non‑zero value
        if (idx % 5 == 0) {
            wgt[idx] = 1.0f + static_cast<float>(idx);
        } else {
            wgt[idx] = 0.0f;
        }
        idx = idx + 1;
    }
}

// Helper: reset accumulator vectors to zero
void clear_accumulators(float accG[], float accU[], int dim) {
    for (int i = 0; i < dim; ++i) {
        accG[i] = 0.0f;
        accU[i] = 0.0f;
    }
}

// Helper: produce a deterministic gradient for demonstration
void fake_gradient(const float wgt[], float grad[], int dim) {
    int i = 0;
    while (i < dim) {
        // simple quadratic loss gradient: dL/dw = 2 * (w - target)
        float target = 5.0f;                     // pretend target value
        float diff    = wgt[i] - target;
        grad[i] = 2.0f * diff;                   // gradient component
        i = i + 1;
    }
}

// Core: one Adadelta update step (expanded multi‑step math)
void adadelta_update(float wgt[], float grad[],
                     float accG[], float accU[],
                     int dim, float rho, float eps) {
    int j = 0;
    while (j < dim) {
        // 1) update running average of squared gradients
        float grad_sq   = grad[j] * grad[j];
        float newAccG   = rho * accG[j] + (1.0f - rho) * grad_sq;

        // 2) compute RMS of previous updates and current gradient
        float rmsU_prev = std::sqrt(accU[j] + eps);
        float rmsG_cur  = std::sqrt(newAccG + eps);

        // 3) calculate parameter update (Δw)
        float raw_step = - (rmsU_prev / rmsG_cur) * grad[j];

        // 4) update running average of squared updates
        float step_sq   = raw_step * raw_step;
        float newAccU   = rho * accU[j] + (1.0f - rho) * step_sq;

        // 5) apply update to weight
        wgt[j] = wgt[j] + raw_step;

        // 6) store new accumulator values
        accG[j] = newAccG;
        accU[j] = newAccU;

        j = j + 1;
    }
}

// --------------------------------------------------
int main() {
    // Problem dimension (expanded for sparsity)
    int dim = 20;

    // Stack allocation of all vectors
    float wgt[20];
    float grad[20];
    float accG[20];
    float accU[20];

    // Hyper‑parameters (float only)
    float rho = 0.95f;          // decay factor
    float eps = 1e-6f;          // stability term
    int   steps = 10;           // number of optimization iterations

    // Initialise structures
    seed_weights(wgt, dim);
    clear_accumulators(accG, accU, dim);

    // Optimization loop (reordered control flow)
    int iter = 0;
    while (iter < steps) {
        // compute deterministic gradient based on current weights
        fake_gradient(wgt, grad, dim);

        // perform a single Adadelta step
        adadelta_update(wgt, grad, accG, accU, dim, rho, eps);

        iter = iter + 1;
    }

    // Output final weights
    std::cout << "Final weights after " << steps << " Adadelta steps:\n";
    for (int k = 0; k < dim; ++k) {
        std::cout << "w[" << k << "] = " << wgt[k] << "\n";
    }

    return 0;
}
