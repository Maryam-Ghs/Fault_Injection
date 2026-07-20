#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

int main()
{
    // ------------------------------------------------------------
    // Version #8 – Adaptive‑step Runge–Kutta (RK4) – all inside main()
    // ------------------------------------------------------------

    // ----- deterministic test vector ---------------------------------
    // ODE: dy/dx = y             (exact solution: y = exp(x))
    // initial condition: x0 = 0 , y0 = 1
    // integrate to xTarget = 5
    // ------------------------------------------------------------

    float xStart   = 0.0f;
    float yStart   = 1.0f;
    float xTarget  = 5.0f;

    // tolerance for adaptive stepping (absolute)
    float eps       = 5.0e-5f;

    // initial step size
    float hGuess    = 0.457f;

    // maximum number of points we may store (worst case many tiny steps)
    int   maxPoints = 20000;

    // allocate storage on the heap
    float* xs = new float[maxPoints];
    float* ys = new float[maxPoints];

    // store the first point
    xs[0] = xStart;
    ys[0] = yStart;

    // counters
    int   idx   = 0;          // index of the last stored point
    float xNow  = xStart;
    float yNow  = yStart;
    float hNow  = hGuess;

    // ----------------------------------------------------------------
    // Helper lambda for the ODE right‑hand side (allowed inside main)
    // ----------------------------------------------------------------
    auto rhs = [](float xv, float yv) -> float
    {
        return yv;                     // dy/dx = y
    };

    // ----------------------------------------------------------------
    // Adaptive stepping loop
    // ----------------------------------------------------------------
    while (xNow < xTarget)
    {
        // ensure we do not step beyond the target
        if (xNow + hNow > xTarget) hNow = xTarget - xNow;

        // ---- single RK4 step of size hNow -------------------------
        float k1 = rhs(xNow,                 yNow);
        float k2 = rhs(xNow + 0.5f*hNow,     yNow + 0.5f*hNow*k1);
        float k3 = rhs(xNow + 0.5f*hNow,     yNow + 0.5f*hNow*k2);
        float k4 = rhs(xNow + hNow,          yNow + hNow*k3);

        float yBig = yNow + (hNow/6.0f)*(k1 + 2.0f*k2 + 2.0f*k3 + k4);

        // ---- two RK4 steps of size hNow/2 -------------------------
        float hHalf = 0.5f * hNow;

        // first half‑step
        float k1a = rhs(xNow,                     yNow);
        float k2a = rhs(xNow + 0.5f*hHalf,        yNow + 0.5f*hHalf*k1a);
        float k3a = rhs(xNow + 0.5f*hHalf,        yNow + 0.5f*hHalf*k2a);
        float k4a = rhs(xNow + hHalf,             yNow + hHalf*k3a);
        float yMid = yNow + (hHalf/6.0f)*(k1a + 2.0f*k2a + 2.0f*k3a + k4a);

        // second half‑step (starting from the middle)
        float k1b = rhs(xNow + hHalf,            yMid);
        float k2b = rhs(xNow + hHalf + 0.5f*hHalf,
                        yMid + 0.5f*hHalf*k1b);
        float k3b = rhs(xNow + hHalf + 0.5f*hHalf,
                        yMid + 0.5f*hHalf*k2b);
        float k4b = rhs(xNow + hNow,             yMid + hHalf*k3b);
        float ySmall = yMid + (hHalf/6.0f)*(k1b + 2.0f*k2b + 2.0f*k3b + k4b);

        // ---- error estimate (difference between the two approaches) --
        float err = fabsf(ySmall - yBig);

        // ---- decide whether to accept the step ----------------------
        if (err <= eps)                     // step is good enough
        {
            // advance the solution
            xNow += hNow;
            yNow = ySmall;                  // the more accurate value

            // store the accepted point
            ++idx;
            if (idx >= maxPoints) break;    // safety guard
            xs[idx] = xNow;
            ys[idx] = yNow;
        }

        // ---- compute the next step size (simple proportional control) --
        // avoid division by zero
        float safety = 0.9f;
        float factor = safety * powf(eps / (err + 1e-12f), 0.25f);
        // limit the factor to avoid extreme changes
        if (factor < 0.1f) factor = 0.1f;
        if (factor > 4.0f) factor = 4.0f;
        hNow = hNow * factor;

        // keep step size positive
        if (hNow < 1e-6f) hNow = 1e-6f;
    }

    // ----------------------------------------------------------------
    // Print the results
    // ----------------------------------------------------------------
    std::cout << "Adaptive RK4 integration results (float precision)\n";
    std::cout << "x\t\t y_numeric\t y_exact\t error\n";

    for (int i = 0; i <= idx; ++i)
    {
        float xVal = xs[i];
        float yNum = ys[i];
        // exact solution of the test ODE
        float yExa = expf(xVal);
        float diff = fabsf(yNum - yExa);
        std::cout << xVal << "\t " << yNum << "\t " << yExa << "\t " << diff << "\n";
    }

    // clean up heap memory
    delete [] xs;
    delete [] ys;

    return 0;
}
