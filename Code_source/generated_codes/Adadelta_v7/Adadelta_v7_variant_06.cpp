/* LLM input variant 6: ordered-structured */
#include <iostream>
#include <cmath>

// Adadelta optimizer version #7
// --------------------------------------------------
// Helper: fill weight vector with a descending ordered pattern
void seed_weights(float wgt[], int dim) {
    for (int i = 0; i < dim; ++i) {
        wgt[i] = static_cast<float>(dim - i); // e.g., dim=10 -> 10,9,...,1
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
    for (int i = 0; i < dim; ++i) {
        // simple quadratic loss gradient: dL/dw = 2 * (w - target)
        float target = 5.0f;                     // pretend target value
        float diff    = wgt[i] - target;
        grad[i] = 2.0f * diff;                   // gradient component
    }
}

// Core: one Adadelta update step (expanded multi‑step math)
void adadelta_update(float wgt[], float grad[],
                     float accG[], float accU[],
                     int dim, float rho, float eps) {
    for (int j = 0; j < dim; ++j) {
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
    }
}

// --------------------------------------------------
int main() {
    // Problem dimension (ordered structured array)
    const int dim = 10;

    // Stack allocation of all vectors
    float wgt[dim];
    float grad[dim];
    float accG[dim];
    float accU[dim];

    // Hyper‑parameters (float only)
    float rho = 0.95f;          // decay factor
    float eps = 1e-6f;          // stability term
    int   steps = 12;           // number of optimization iterations

    // Initialise structures
    seed_weights(wgt, dim);
    clear_accumulators(accG, accU, dim);

    // Optimization loop (reordered control flow)
    for (int iter = 0; iter < steps; ++iter) {
        // compute deterministic gradient based on current weights
        fake_gradient(wgt, grad, dim);

        // perform a single Adadelta step
        adadelta_update(wgt, grad, accG, accU, dim, rho, eps);
    }

    // Output final weights
    std::cout << "Final weights after " << steps << " Adadelta steps:\n";
    for (int k = 0; k < dim; ++k) {
        std::cout << "w[" << k << "] = " << wgt[k] << "\n";
    }

    return 0;
}
