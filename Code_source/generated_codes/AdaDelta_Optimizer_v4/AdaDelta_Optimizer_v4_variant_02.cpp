#include <iostream>
#include <cmath>

/* LLM input variant 2: small-diverse */

#define DIM 12          // dimension of the parameter vector
#define STEPS 7         // number of AdaDelta updates

int main() {
    // ---------- deterministic initial parameters ----------
    float param[DIM] = {
        0.10f, -0.20f, 0.35f, -0.45f,
        0.55f, -0.65f, 0.75f, -0.85f,
        0.95f, -0.05f, 0.15f, -0.25f
    };

    // ---------- deterministic gradients for each step ----------
    const float gradSteps[STEPS][DIM] = {
        { 0.12f, -0.22f, 0.33f, -0.44f, 0.51f, -0.61f, 0.71f, -0.81f, 0.91f, -0.01f, 0.11f, -0.21f },
        { -0.15f, 0.25f, -0.35f, 0.45f, -0.55f, 0.65f, -0.75f, 0.85f, -0.95f, 0.05f, -0.15f, 0.25f },
        { 0.18f, -0.28f, 0.38f, -0.48f, 0.58f, -0.68f, 0.78f, -0.88f, 0.98f, -0.08f, 0.18f, -0.28f },
        { -0.21f, 0.31f, -0.41f, 0.51f, -0.61f, 0.71f, -0.81f, 0.91f, -0.01f, 0.11f, -0.21f, 0.31f },
        { 0.24f, -0.34f, 0.44f, -0.54f, 0.64f, -0.74f, 0.84f, -0.94f, 0.04f, -0.14f, 0.24f, -0.34f },
        { -0.27f, 0.37f, -0.47f, 0.57f, -0.67f, 0.77f, -0.87f, 0.97f, -0.07f, 0.03f, -0.13f, 0.23f },
        { 0.30f, -0.40f, 0.50f, -0.60f, 0.70f, -0.80f, 0.90f, -0.02f, 0.08f, -0.18f, 0.28f, -0.38f }
    };

    // ---------- AdaDelta state (accumulated squared grads and deltas) ----------
    float accGrad[DIM] = {0.0f};
    float accDelta[DIM] = {0.0f};

    // hyper‑parameters (no const per the constraints)
    float rho = 0.95f;
    float eps = 1e-6f;

    // ---------- perform a few optimization steps ----------
    int step = 0;
    while (step < STEPS) {
        // use deterministic gradient for this step
        for (int i = 0; i < DIM; ++i) {
            // copy from pre‑defined table
            // grad is a temporary variable, no need to store long‑term
            // using a single array to hold current gradient
            // (reusing the same array each iteration)
            // we will define it here
            // Note: we could also declare grad outside loop, but this keeps scope clear
        }
        float grad[DIM];
        for (int i = 0; i < DIM; ++i) {
            grad[i] = gradSteps[step][i];
        }

        // update each dimension
        int idx = 0;
        while (idx < DIM) {
            // 1) update running average of squared gradients
            float g2 = grad[idx] * grad[idx];
            float tmp1 = rho * accGrad[idx];
            float tmp2 = (1.0f - rho) * g2;
            accGrad[idx] = tmp1 + tmp2;

            // 2) compute RMS of accumulated grads
            float rmsGrad = std::sqrt(accGrad[idx] + eps);

            // 3) compute RMS of accumulated deltas (from previous step)
            float rmsDelta = std::sqrt(accDelta[idx] + eps);

            // 4) compute adaptive learning rate factor
            float factor = rmsDelta / rmsGrad;

            // 5) compute parameter update (delta)
            float delta = -factor * grad[idx];

            // 6) apply update
            param[idx] = param[idx] + delta;

            // 7) update running average of squared deltas
            float d2 = delta * delta;
            float tmp3 = rho * accDelta[idx];
            float tmp4 = (1.0f - rho) * d2;
            accDelta[idx] = tmp3 + tmp4;

            ++idx;                               // advance to next dimension
        }

        ++step;                                   // next optimization step
    }

    // ---------- output final parameters ----------
    std::cout << "AdaDelta final parameters (version #4):\n";
    for (int i = 0; i < DIM; ++i) {
        std::cout << param[i] << (i + 1 == DIM ? '\n' : ' ');
    }
    return 0;
}
