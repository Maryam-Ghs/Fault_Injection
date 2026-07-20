// Adaptive Step Runge–Kutta implementation – version #6
// Only float and int are used. All data are kept on the stack.
/* LLM input variant 9: medium-deterministic-random */

#include <cstdio>
#include <cmath>

// ------------------------------------------------------------
// System of ODEs (example: stiff problem)
//    dy1/dx = -1000*y1 + 3000 - 2000*exp(-x)
//    dy2/dx =  y1 - y2
// ------------------------------------------------------------
void rhs(float x, float y[2], float dydx[2])
{
    // compute exp(-x) using float arithmetic
    float e = expf(-x);
    dydx[0] = -1000.0f * y[0] + 3000.0f - 2000.0f * e;
    dydx[1] = y[0] - y[1];
}

// ------------------------------------------------------------
// One classical RK4 step (single step of size h)
// ------------------------------------------------------------
void rk4_one(float x, float y[2], float h, float yout[2])
{
    float k1[2], k2[2], k3[2], k4[2];
    float yt[2];

    // k1
    rhs(x, y, k1);

    // k2
    yt[0] = y[0] + 0.5f * h * k1[0];
    yt[1] = y[1] + 0.5f * h * k1[1];
    rhs(x + 0.5f * h, yt, k2);

    // k3
    yt[0] = y[0] + 0.5f * h * k2[0];
    yt[1] = y[1] + 0.5f * h * k2[1];
    rhs(x + 0.5f * h, yt, k3);

    // k4
    yt[0] = y[0] + h * k3[0];
    yt[1] = y[1] + h * k3[1];
    rhs(x + h, yt, k4);

    // combine
    yout[0] = y[0] + (h / 6.0f) * (k1[0] + 2.0f * k2[0] + 2.0f * k3[0] + k4[0]);
    yout[1] = y[1] + (h / 6.0f) * (k1[1] + 2.0f * k2[1] + 2.0f * k3[1] + k4[1]);
}

// ------------------------------------------------------------
// RK4 step with error estimate (full step vs two half steps)
// ------------------------------------------------------------
void rk4_step_err(float x, float y[2], float h, float yfull[2], float err[2])
{
    float ytemp[2];
    // Full step
    rk4_one(x, y, h, yfull);

    // Two half steps
    rk4_one(x, y, 0.5f * h, ytemp);
    rk4_one(x + 0.5f * h, ytemp, 0.5f * h, ytemp);

    // Error estimate (difference between full and half-step results)
    err[0] = ytemp[0] - yfull[0];
    err[1] = ytemp[1] - yfull[1];
}

// ------------------------------------------------------------
// Maximum absolute error of the vector
// ------------------------------------------------------------
float max_abs_err(float err[2])
{
    float m0 = (err[0] >= 0.0f) ? err[0] : -err[0];
    float m1 = (err[1] >= 0.0f) ? err[1] : -err[1];
    return (m0 > m1) ? m0 : m1;
}

// ------------------------------------------------------------
// Adjust step size based on error and tolerance
// ------------------------------------------------------------
float adapt_h(float h, float errmax, float tol)
{
    if (errmax > tol)               // error too large → shrink
        h = h * 0.5f;
    else if (errmax < tol * 0.25f)  // error tiny → grow
        h = h * 2.0f;
    // keep h unchanged otherwise
    // guard against too small step
    if (h < 1e-6f) h = 1e-6f;
    return h;
}

// ------------------------------------------------------------
// Main driver – generates its own input and prints results
// ------------------------------------------------------------
int main()
{
    // Problem definition (deterministic pseudo‑random input)
    float x_start = 0.0f;
    float x_stop  = 1.2345f;          // longer interval, still modest
    float y_state[2] = { 1.234f, -0.567f }; // non‑zero initial conditions
    float h_cur = 0.07f;               // initial step size
    float tolerance = 5e-4f;           // tighter accuracy requirement

    // Header
    std::printf("   x          y1           y2\n");
    std::printf("%8.4f %12.6f %12.6f\n", x_start, y_state[0], y_state[1]);

    // Integration loop
    int iter = 0;
    while (x_start < x_stop && iter < 10000)
    {
        // Ensure we do not step beyond the domain
        if (x_start + h_cur > x_stop)
            h_cur = x_stop - x_start;

        float y_full[2];
        float err_vec[2];

        // Perform step with error estimate
        rk4_step_err(x_start, y_state, h_cur, y_full, err_vec);

        // Compute maximum error component
        float emax = max_abs_err(err_vec);

        // Adapt step size for next iteration
        h_cur = adapt_h(h_cur, emax, tolerance);

        // Accept step if error within tolerance
        if (emax <= tolerance)
        {
            x_start += h_cur;          // advance independent variable
            y_state[0] = y_full[0];
            y_state[1] = y_full[1];
            std::printf("%8.4f %12.6f %12.6f\n", x_start, y_state[0], y_state[1]);
        }
        // otherwise repeat with reduced step (h_cur already reduced)

        ++iter;
    }

    // End of integration
    std::printf("Integration finished after %d iterations.\n", iter);
    return 0;
}
