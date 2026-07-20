/*  Adams–Moulton Method implementation – version #7  */
/* LLM input variant 6: ordered-structured */
#include <cstdio>
#include <cstdlib>
#include <ctime>

int main()
{
    /* ---------- configuration (deterministic, ordered) ---------- */
    // Fixed seed not required; values are chosen to be structured and sorted.
    int maxSteps = 20;                                 /* stack capacity */
    int stepsCnt = 10;                                 /* 10 steps, within capacity */
    float step_h  = 0.2f;                               /* constant step size */
    float param_a = 1.5f;                               /* constant parameter a */
    float start_y = 3.0f;                               /* initial y value */

    /* ---------- storage on the stack ---------- */
    float timeArr[20];
    float solArr[20];
    float fArr[20];                /* f(t,y) = -a*y  */

    /* ---------- initial condition ---------- */
    timeArr[0] = 0.0f;
    solArr[0]  = start_y;
    fArr[0]    = -param_a * start_y;

    /* ---------- verbose Adams–Moulton (trapezoidal) loop ---------- */
    int idx = 0;
    while ( idx < stepsCnt )
    {
        /* current values */
        float t_cur   = timeArr[idx];
        float y_cur   = solArr[idx];
        float f_cur   = fArr[idx];

        /* predict next time */
        float t_next = t_cur + step_h;

        /* implicit trapezoidal formula (exact for linear ODE) */
        /* y_next * (1 + a*h/2) = y_cur * (1 - a*h/2) */
        float numerator   = y_cur * ( 1.0f - param_a * step_h * 0.5f );
        float denominator = 1.0f + param_a * step_h * 0.5f;
        float y_next      = numerator / denominator;

        /* evaluate derivative at new point */
        float f_next = -param_a * y_next;

        /* store results */
        timeArr[idx + 1] = t_next;
        solArr [idx + 1] = y_next;
        fArr   [idx + 1] = f_next;

        /* advance index (changed loop structure) */
        idx = idx + 1;
    }

    /* ---------- output ---------- */
    std::printf( "Adams–Moulton (trapezoidal) integration\n" );
    std::printf( "Parameters: a = %.3f , h = %.3f , steps = %d\n\n",
                 param_a, step_h, stepsCnt );

    std::printf( "   t          y(t)\n" );
    std::printf( "---------------------------\n" );
    for ( int j = 0; j <= stepsCnt; ++j )
    {
        std::printf( "%6.3f   %10.6f\n", timeArr[j], solArr[j] );
    }

    return 0;
}
