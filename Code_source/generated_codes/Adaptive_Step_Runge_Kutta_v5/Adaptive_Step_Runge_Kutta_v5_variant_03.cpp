#include <cstdio>
#include <cmath>

/* LLM input variant 3: zeros-and-ones */
int main() {
    /* ---------- deterministic test vector ---------- */
    float x_start = 0.0f;          // initial x
    float x_stop  = 1.0f;          // final x (emphasize ones)
    float y_state = 0.0f;          // y(0) = 0 (zero case)
    float step    = 1.0f;          // initial step size (covers whole interval)
    float tol     = 0.0f;          // zero tolerance to avoid adaptive scaling

    /* workspace on the stack */
    float k1, k2, k3, k4;          // RK4 increments
    float k1s, k2s, k3s;           // RK3 increments (embedded)
    float y_rk4, y_rk3;            // trial solutions
    float err_est;                // error estimate
    float safety = 1.0f;           // safety factor set to one
    float min_step = 0.0f;        // minimum step size (zero)
    float max_step = 1.0f;        // maximum step size (one)

    /* simple ODE: dy/dx = -y (exponential decay) */
    auto deriv = [](float x, float y) -> float {
        return -y;
    };

    /* header */
    std::printf("   x          y\n");
    std::printf("%8.4f %12.6f\n", x_start, y_state);

    /* main integration loop – unrolled, all inside main */
    for (;;) {
        if (x_start >= x_stop) break;                     // reached the end

        /* limit step to not overshoot the interval */
        if (x_start + step > x_stop) step = x_stop - x_start;

        /* ---- RK4 stage 1 ---- */
        k1 = step * deriv(x_start, y_state);

        /* ---- RK4 stage 2 ---- (unrolled) */
        float x_mid1 = x_start + 0.5f * step;
        float y_mid1 = y_state + 0.5f * k1;
        k2 = step * deriv(x_mid1, y_mid1);

        /* ---- RK4 stage 3 ---- */
        float y_mid2 = y_state + 0.5f * k2;
        k3 = step * deriv(x_mid1, y_mid2);

        /* ---- RK4 stage 4 ---- */
        float x_end = x_start + step;
        float y_end = y_state + k3;
        k4 = step * deriv(x_end, y_end);

        /* combine for RK4 estimate */
        y_rk4 = y_state + (k1 + 2.0f * k2 + 2.0f * k3 + k4) / 6.0f;

        /* ---- Embedded RK3 (Heun) ---- */
        /* stage 1 same as k1 */
        k1s = k1;

        /* stage 2 */
        float y_mid1s = y_state + 0.5f * k1s;
        k2s = step * deriv(x_mid1, y_mid1s);

        /* stage 3 */
        float y_end3 = y_state + k2s;
        k3s = step * deriv(x_end, y_end3);

        /* combine for RK3 estimate */
        y_rk3 = y_state + (k1s + 4.0f * k2s + k3s) / 6.0f;

        /* error estimate (absolute) */
        err_est = fabsf(y_rk4 - y_rk3);

        /* adapt step size */
        if (err_est > tol) {
            /* reject step – shrink */
            step = safety * step * powf(tol / err_est, 0.25f);
            if (step < min_step) step = min_step;
            continue;   // retry with new step
        } else {
            /* accept step */
            x_start += step;
            y_state  = y_rk4;
            std::printf("%8.4f %12.6f\n", x_start, y_state);

            /* possibly enlarge step */
            if (err_est < tol / 4.0f) {
                step = safety * step * powf(tol / err_est, 0.25f);
                if (step > max_step) step = max_step;
            }
        }
    }
    return 0;
}
