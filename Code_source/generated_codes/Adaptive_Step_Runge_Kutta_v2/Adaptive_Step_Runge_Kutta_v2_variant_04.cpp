#include <cstdio>
#include <cstdlib>
#include <cmath>

#define MAXSTP 20000
#define TOLER 1e-4f
#define HINIT 0.2f
#define XEND  10.0f

/* LLM input variant 4: signed-extremes */

int main()
{
    // -------------------------------------------------
    // generate a deterministic array of initial conditions
    // mixing negative, zero, and positive values
    // -------------------------------------------------
    float initX[MAXSTP];
    float initY[MAXSTP];
    for (int i = 0; i < MAXSTP; ++i)
    {
        // cycle through -2, -1, 0, 1, 2 for x
        initX[i] = (float)((i % 5) - 2); // values: -2, -1, 0, 1, 2

        // cycle through -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5 for y
        initY[i] = 0.5f * (float)(((i % 7) - 3)); // values: -1.5 .. 1.5
    }

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
