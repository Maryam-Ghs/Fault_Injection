#include <cstdio>
#include <cstdlib>
#include <cmath>

/* LLM input variant 2: small-diverse */
#define MAXSTP 10
#define TOLER 1e-4f
#define HINIT 0.2f
#define XEND  5.0f

int main()
{
    // -------------------------------------------------
    // generate a small deterministic array of initial conditions
    // -------------------------------------------------
    float initX[MAXSTP] = {0.0f, 0.5f, 0.6f, 0.2f, 0.9f, 0.3f, 0.7f, 0.8f, 0.1f, 0.4f};
    float initY[MAXSTP] = {0.0f, 1.0f, 1.5f, 0.2f, 1.8f, 0.6f, 1.2f, 1.6f, 0.3f, 0.9f};

    // -------------------------------------------------
    // adaptive Runge–Kutta (RK4 with embedded RK2 error)
    // everything lives on the stack
    // -------------------------------------------------
    float xs[MAXSTP];
    float ys[MAXSTP];
    int   cnt = 0;                     // number of accepted steps
    float x  = initX[0];               // start from first deterministic point
    float y  = initY[0];               // start from first deterministic point
    float h  = HINIT;                  // initial step size

    while (x < XEND && cnt < MAXSTP - 1)
    {
        // ----- stage 1 -----
        float f1 = -y + sinf(x);                     // dy/dx at (x,y)

        // ----- stage 2 (using half step for embedded method) -----
        float xh = x + 0.5f * h;
        float yh = y + 0.5f * h * f1;
        float f2 = -yh + sinf(xh);

        // ----- stage 3 -----
        float xh2 = x + 0.5f * h;
        float yh2 = y + 0.5f * h * f2;
        float f3 = -yh2 + sinf(xh2);

        // ----- stage 4 -----
        float xe = x + h;
        float ye = y + h * f3;
        float f4 = -ye + sinf(xe);

        // ----- RK4 update (full order) -----
        float incr4 = (h / 6.0f) * (f1 + 2.0f * f2 + 2.0f * f3 + f4);
        float y4 = y + incr4;

        // ----- RK2 update (lower order) -----
        float incr2 = (h / 2.0f) * (f1 + f2);
        float y2 = y + incr2;

        // ----- error estimate -----
        float err = fabsf(y4 - y2);

        // ----- adapt step size -----
        if (err <= TOLER)                     // accept step
        {
            x = xe;                           // advance to end of interval
            y = y4;                           // use higher order result
            xs[cnt] = x;
            ys[cnt] = y;
            ++cnt;
        }

        // scale step for next iteration (always keep it positive)
        float scale = 0.9f * powf(TOLER / (err + 1e-7f), 0.25f);
        if (scale < 0.1f) scale = 0.1f;
        if (scale > 4.0f) scale = 4.0f;
        h = h * scale;

        // prevent overshooting the domain
        if (x + h > XEND) h = XEND - x;
        if (h < 1e-5f) h = 1e-5f;
    }

    // -------------------------------------------------
    // print the trajectory
    // -------------------------------------------------
    for (int i = 0; i < cnt; ++i)
    {
        printf("%8.4f %12.6f\n", xs[i], ys[i]);
    }

    return 0;
}
