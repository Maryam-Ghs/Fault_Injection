// Adaptive Control implementation – version #8
/* LLM input variant 2: small-diverse */
// ------------------------------------------------
// This program simulates a simple discrete‑time adaptive controller.
// All code resides inside main(), uses only float/int, std::vector and
// deterministic test vectors. No external input is read.

#include <vector>
#include <cstdio>
#include <cmath>

int main()
{
    // ------------------------------------------------
    // Simulation settings
    // ------------------------------------------------
    int steps =  twenty;                    // total simulation steps
    float a_real = 0.6f;                 // true plant coefficient a
    float b_real = 0.7f;                 // true plant coefficient b
    float a_ref  = 0.5f;                 // reference model coefficient a
    float b_ref  = 0.4f;                 // reference model coefficient b
    float adapt_gain = 0.02f;            // adaptation gain γ

    // ------------------------------------------------
    // Containers for signals and parameters
    // ------------------------------------------------
    std::vector<float> r(steps);         // reference signal
    std::vector<float> y(steps);         // plant output
    std::vector<float> ym(steps);        // model output
    std::vector<float> u(steps);         // control input
    std::vector<float> th1(steps);       // adaptive weight for r
    std::vector<float> th2(steps);       // adaptive weight for y

    // ------------------------------------------------
    // Initial conditions
    // ------------------------------------------------
    r[0]  = 0.5f;
    y[0]  = 0.2f;
    ym[0] = 0.0f;
    th1[0] = 0.1f;
    th2[0] = -0.1f;

    // ------------------------------------------------
    // Deterministic reference trajectory (ramp + sinusoid)
    // ------------------------------------------------
    for (int i = 1; i < steps; ++i)
    {
        if (i < 5)
            r[i] = 0.5f + 0.1f * static_cast<float>(i);          // ramp part
        else
            r[i] = 0.8f * sinf(0.2f * static_cast<float>(i)) + 0.2f; // sinusoid part
    }

    // ------------------------------------------------
    // Main adaptive loop (while‑style, heavily iterative)
    // ------------------------------------------------
    int k = 0;
    while (k < steps - 1)
    {
        // ---- Control law (current parameters) ----
        float ctrl = th1[k] * r[k] + th2[k] * y[k];
        u[k] = ctrl;

        // ---- Plant update (true dynamics) ----
        float y_next = a_real * y[k] + b_real * ctrl;
        y[k + 1] = y_next;

        // ---- Reference model update ----
        float ym_next = a_ref * ym[k] + b_ref * r[k];
        ym[k + 1] = ym_next;

        // ---- Error between plant and model ----
        float err = y[k + 1] - ym[k + 1];

        // ---- Regressor vector (expanded steps) ----
        float phi_r = r[k];          // regressor for reference
        float phi_y = y[k];          // regressor for plant output

        // ---- Parameter adaptation (multi‑step arithmetic) ----
        float delta_th1 = -adapt_gain * err * phi_r;
        float delta_th2 = -adapt_gain * err * phi_y;

        float th1_tmp = th1[k] + delta_th1;
        float th2_tmp = th2[k] + delta_th2;

        // ---- Store updated parameters ----
        th1[k + 1] = th1_tmp;
        th2[k + 1] = th2_tmp;

        // ---- Advance time index (reordered increment) ----
        k = k + 1;
    }

    // ------------------------------------------------
    // Output a subset of the simulation results
    // ------------------------------------------------
    for (int i = 0; i < steps; i += 5)
    {
        printf(
            "Step %2d | r=%.3f | y=%.3f | ym=%.3f | u=%.3f | th1=%.3f | th2=%.3f\n",
            i, r[i], y[i], ym[i], u[i], th1[i], th2[i]);
    }

    return 0;
}
