// LLM input variant 4: signed-extremes
// Adaptive Control - Version 5
// Simple MRAC example using only int and float types.
// Generates its own deterministic mixed-sign data, runs on stack arrays and prints results.

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

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
    // Seed random number generator (kept for compatibility, not used)
    std::srand(static_cast<unsigned>(std::time(0)));

    const int N = 1000;                     // size of simulation (large array)
    float ref_sig[N];                       // reference signal r[t]
    float meas_out[N];                      // measured plant output y[t]
    float ctrl_sig[N];                      // control signal u[t]

    // Plant constants (unknown to controller)
    const float a = 0.7f;                   // y[t] = a*y[t-1] + b*u[t-1] + noise
    const float b = 0.5f;
    const float noise_amp = 0.05f;

    // Initialise arrays with zeros
    for (int i = 0; i < N; ++i) {
        ref_sig[i] = 0.0f;
        meas_out[i] = 0.0f;
        ctrl_sig[i] = 0.0f;
    }

    // Fill reference signal with a deterministic mix of negative, zero, and positive values
    for (int i = 0; i < N; ++i) {
        if (i % 3 == 0) {
            ref_sig[i] = -1.0f;               // negative extreme
        } else if (i % 3 == 1) {
            ref_sig[i] = 0.0f;                // zero
        } else {
            ref_sig[i] = 1.0f;                 // positive extreme
        }
    }

    AdaptiveCtrl controller;

    // Simulation loop
    int t = 1;                      // start from 1 because we need y[t-1]
    while (t < N) {
        // 1) compute control based on current reference and previous output
        float u_val = controller.compute(ref_sig[t - 1], meas_out[t - 1]);
        ctrl_sig[t] = u_val;

        // 2) plant dynamics (simple first‑order with deterministic noise)
        // Alternate noise between -0.5*noise_amp and +0.5*noise_amp
        float noise = ((t % 2 == 0) ? -0.5f : 0.5f) * noise_amp;
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

    // Print first 10 samples of reference, output and control
    std::cout << "First 10 samples (t, r[t], y[t], u[t]):\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << i << "  " << ref_sig[i] << "  " << meas_out[i] << "  " << ctrl_sig[i] << "\n";
    }

    return 0;
}
