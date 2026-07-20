/* LLM input variant 1: minimal-boundary */
// Adaptive Control - Version 5
// Simple MRAC example using only int and float types.
// Generates its own deterministic minimal data, runs on stack arrays and prints results.

#include <iostream>
#include <cmath>
#include <algorithm>

class AdaptiveCtrl {
    // Adaptive parameters
    float k_ref;   // gain for reference signal
    float k_out;   // gain for measured output
    float learn;   // learning rate (gamma)

public:
    AdaptiveCtrl()
        : k_ref(0.0f), k_out(0.0f), learn(0.001f) {}

    // Compute control signal u[t] = k_ref * r[t] + k_out * y[t]
    float compute(float r_val, float y_val) {
        // expanded multi‑step computation
        float term1 = k_ref * r_val;
        float term2 = k_out * y_val;
        float u_val = term1 + term2;
        return u_val;
    }

    // Update parameters using gradient descent:
    // k = k - learn * e * phi   (phi = [r, y])
    void adapt(float r_val, float y_val, float err) {
        // reorder: compute product first, then update
        float grad_ref = err * r_val;
        float grad_out = err * y_val;

        // multi‑step update
        float delta_ref = learn * grad_ref;
        float delta_out = learn * grad_out;

        k_ref = k_ref - delta_ref;
        k_out = k_out - delta_out;
    }

    // Accessors for final reporting
    float getKref() const { return k_ref; }
    float getKout() const { return k_out; }
};

int main() {
    const int N = 2;                     // minimal simulation size (nontrivial)
    float ref_sig[N];                    // reference signal r[t]
    float meas_out[N];                   // measured plant output y[t]
    float ctrl_sig[N];                   // control signal u[t]

    // Plant constants (unknown to controller)
    const float a = 0.7f;                // y[t] = a*y[t-1] + b*u[t-1] + noise
    const float b = 0.5f;
    const float noise_amp = 0.05f;

    // Initialise arrays with deterministic values
    ref_sig[0] = 0.5f;
    ref_sig[1] = -0.5f;
    meas_out[0] = 0.0f;
    meas_out[1] = 0.0f;
    ctrl_sig[0] = 0.0f;
    ctrl_sig[1] = 0.0f;

    AdaptiveCtrl controller;

    // Simulation loop
    int t = 1;                      // start from 1 because we need y[t-1]
    while (t < N) {
        // 1) compute control based on current reference and previous output
        float u_val = controller.compute(ref_sig[t - 1], meas_out[t - 1]);
        ctrl_sig[t] = u_val;

        // 2) plant dynamics (deterministic, zero noise)
        float noise = 0.0f; // deterministic minimal noise
        float y_val = a * meas_out[t - 1] + b * u_val + noise;
        meas_out[t] = y_val;

        // 3) error between plant output and reference
        float err = y_val - ref_sig[t];

        // 4) adapt controller parameters
        controller.adapt(ref_sig[t], y_val, err);

        ++t;
    }

    // Print final adaptive gains
    std::cout << "Final adaptive gains after " << N << " steps:\n";
    std::cout << "k_ref (gain on reference) = " << controller.getKref() << "\n";
    std::cout << "k_out (gain on output)   = " << controller.getKout() << "\n\n";

    // Print first min(N,10) samples of reference, output and control
    int limit = std::min(N, 10);
    std::cout << "First " << limit << " samples (t, r[t], y[t], u[t]):\n";
    for (int i = 0; i < limit; ++i) {
        std::cout << i << "  " << ref_sig[i] << "  " << meas_out[i] << "  " << ctrl_sig[i] << "\n";
    }

    return 0;
}
