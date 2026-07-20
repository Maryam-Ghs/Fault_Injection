#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

int main() {
    // ------------------------------------------------------------
    // 1. Hyper‑parameters (all float, no const as required)
    // ------------------------------------------------------------
    float learn_rate = 0.02f;      // α
    float decay1      = 0.9f;      // β1
    float decay2      = 0.999f;    // β2
    float epsilon     = 1e-8f;    // ε

    // ------------------------------------------------------------
    // 2. Synthetic data (large predefined arrays, still safe)
    // ------------------------------------------------------------
    const int sample_cnt = 10000;   // m
    const int feat_cnt   = 1000;    // n

    // Allocate feature matrix X (m × n) on the heap
    float* X = new float[sample_cnt * feat_cnt];
    // Fill X with deterministic values
    // Row‑major layout: X[i * n + j]
    for (int i = 0; i < sample_cnt; ++i) {
        for (int j = 0; j < feat_cnt; ++j) {
            X[i * feat_cnt + j] = static_cast<float>((i + 1) * (j + 2));
        }
    }

    // Allocate target vector y (length m) on the heap
    float* y = new float[sample_cnt];
    for (int i = 0; i < sample_cnt; ++i) {
        y[i] = static_cast<float>(i * 2 + 1);
    }

    // ------------------------------------------------------------
    // 3. Model parameters (weights), first‑order and ∞‑norm moments
    // ------------------------------------------------------------
    float* w = new float[feat_cnt];          // current parameters
    float* m = new float[feat_cnt];          // first‑order moment (biased)
    float* u = new float[feat_cnt];          // exponentially weighted ∞‑norm

    // Initialise everything to zero
    for (int j = 0; j < feat_cnt; ++j) {
        w[j] = 0.0f;
        m[j] = 0.0f;
        u[j] = 0.0f;
    }

    // ------------------------------------------------------------
    // 4. Training loop – a few epochs, verbose step‑by‑step
    // ------------------------------------------------------------
    int epoch = 0;
    int max_epochs = 10;
    // We'll keep β1^t in a separate variable to avoid recomputing pow()
    float decay1_pow = 1.0f;

    while (epoch < max_epochs) {
        // ---- a) Compute predictions and gradients for each sample ----
        // Zero‑accumulate gradient vector (length n)
        float* grad = new float[feat_cnt];
        for (int j = 0; j < feat_cnt; ++j) grad[j] = 0.0f;

        // Loop over samples
        for (int i = 0; i < sample_cnt; ++i) {
            // Compute dot product w·x_i
            float pred = 0.0f;
            for (int j = 0; j < feat_cnt; ++j) {
                pred += w[j] * X[i * feat_cnt + j];
            }
            // Error term (prediction - true)
            float err = pred - y[i];
            // Accumulate gradient (2 * err * x_ij) for MSE loss
            for (int j = 0; j < feat_cnt; ++j) {
                grad[j] += 2.0f * err * X[i * feat_cnt + j];
            }
        } // end sample loop

        // ---- b) Scale gradient by 1/m (mean gradient) ----
        for (int j = 0; j < feat_cnt; ++j) {
            grad[j] = grad[j] / static_cast<float>(sample_cnt);
        }

        // ---- c) AdaMax moment updates --------------------------------
        // Update first‑order moment m_t
        for (int j = 0; j < feat_cnt; ++j) {
            m[j] = decay1 * m[j] + (1.0f - decay1) * grad[j];
        }

        // Update ∞‑norm moment u_t (max operation)
        for (int j = 0; j < feat_cnt; ++j) {
            float abs_g = std::fabs(grad[j]);
            float scaled_prev = decay2 * u[j];
            // max(scaled_prev, |g_t|)
            u[j] = (scaled_prev > abs_g) ? scaled_prev : abs_g;
        }

        // ---- d) Bias‑correction for m_t (β1^t) -----------------------
        decay1_pow = decay1_pow * decay1;   // β1^t
        float bias_corr = 1.0f - decay1_pow; // 1 - β1^t

        // ---- e) Parameter update ------------------------------------
        for (int j = 0; j < feat_cnt; ++j) {
            // Compute step size: α / (1 - β1^t)
            float step_size = learn_rate / bias_corr;
            // Normalise m_t by (u_t + ε)
            float denom = u[j] + epsilon;
            // Final update (note the order: compute numerator first)
            float delta = step_size * (m[j] / denom);
            w[j] = w[j] - delta;
        }

        // Clean up gradient memory for this epoch
        delete[] grad;

        // ---- f) Optional: print progress for each epoch ------------
        std::cout << "Epoch " << (epoch + 1) << " : ";
        for (int j = 0; j < feat_cnt; ++j) {
            std::cout << "w[" << j << "]=" << w[j] << " ";
        }
        std::cout << std::endl;

        ++epoch;
    } // end training loop

    // ------------------------------------------------------------
    // 5. Final result output
    // ------------------------------------------------------------
    std::cout << "\nFinal learned parameters (AdaMax):\n";
    for (int j = 0; j < feat_cnt; ++j) {
        std::cout << "w[" << j << "] = " << w[j] << std::endl;
    }

    // ------------------------------------------------------------
    // 6. Clean up heap allocations
    // ------------------------------------------------------------
    delete[] X;
    delete[] y;
    delete[] w;
    delete[] m;
    delete[] u;

    return 0;
}
