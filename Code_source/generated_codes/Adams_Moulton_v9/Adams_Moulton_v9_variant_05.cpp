#include <iostream>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // ==================== Version #9 ====================
    // Verbose, step‑by‑step Adams‑Moulton (2‑step) solver.
    // All data lives on the stack, only float/int are used.

    // -----------------------------------------------------------------
    // 1) Define edge‑case heavy problem parameters (generated internally)
    // -----------------------------------------------------------------
    int total_steps = 12;          // number of integration points (edge case: small)
    float step_h   = -0.000001f;   // step size (edge case: negative & very small)

    // Edge case handling: if step_h is zero we cannot proceed.
    if (step_h == 0.0f) {
        std::cout << "Step size cannot be zero.\n";
        return 0;
    }

    // -----------------------------------------------------------------
    // 2) Allocate fixed‑size arrays on the stack
    // -----------------------------------------------------------------
    // We allocate a bit more than needed to be safe for edge cases.
    float t_vals[128];
    float y_vals[128];

    // -----------------------------------------------------------------
    // 3) Initial condition (t0, y0)
    // -----------------------------------------------------------------
    t_vals[0] = 0.0f;
    y_vals[0] = 1.0f;   // y(0) = 1

    // -----------------------------------------------------------------
    // 4) Define the ODE: dy/dt = f(t, y) = 0  (duplicate‑heavy derivative)
    // -----------------------------------------------------------------
    // Constant derivative creates many duplicate values.
    auto f = [](float /*t*/, float /*y*/)->float {
        return 0.0f;
    };

    // -----------------------------------------------------------------
    // 5) First step – use explicit Euler as a starter (required for 2‑step)
    // -----------------------------------------------------------------
    float t_prev = t_vals[0];
    float y_prev = y_vals[0];

    float f_prev = f(t_prev, y_prev);
    float t_next = t_prev + step_h;                 // compute next t first
    float y_next = y_prev + step_h * f_prev;        // Euler predictor

    t_vals[1] = t_next;
    y_vals[1] = y_next;

    std::cout << "Step 0: t = " << t_prev << " , y = " << y_prev << "\n";
    std::cout << "Step 1 (Euler start): t = " << t_next << " , y = " << y_next << "\n";

    // -----------------------------------------------------------------
    // 6) Adams‑Moulton 2‑step loop (predictor–corrector)
    // -----------------------------------------------------------------
    int idx = 2;                     // current index we are filling
    while (idx <= total_steps) {
        // ----- Predictor: Adams‑Bashforth 2‑step -----
        t_prev = t_vals[idx-1];
        y_prev = y_vals[idx-1];
        float t_before = t_vals[idx-2];
        float y_before = y_vals[idx-2];

        float f_curr   = f(t_prev,   y_prev);
        float f_old    = f(t_before, y_before);

        // Predictor formula (explicit)
        float t_pred = t_prev + step_h;
        float y_pred = y_prev + (step_h/2.0f)*(3.0f*f_curr - f_old);

        // ----- Corrector: Adams‑Moulton 2‑step (implicit trapezoidal) -----
        // We need f at the predicted point
        float f_pred = f(t_pred, y_pred);

        // Corrector formula (implicit, but we use the predicted f)
        float y_corr = y_prev + (step_h/2.0f)*(f_curr + f_pred);

        // Store results
        t_vals[idx] = t_pred;
        y_vals[idx] = y_corr;

        // Verbose output for this step
        std::cout << "Step " << idx << ": t = " << t_pred
                  << " , y_pred = " << y_pred
                  << " , y_corr = " << y_corr << "\n";

        // Prepare for next iteration
        ++idx;
    }

    // -----------------------------------------------------------------
    // 7) Final summary
    // -----------------------------------------------------------------
    std::cout << "\n--- Final values ---\n";
    for (int i = 0; i <= total_steps; ++i) {
        std::cout << "i = " << i << " : t = " << t_vals[i]
                  << " , y = " << y_vals[i] << "\n";
    }

    return 0;
}
