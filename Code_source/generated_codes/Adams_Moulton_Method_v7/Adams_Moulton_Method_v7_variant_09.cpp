/* LLM input variant 9: medium-deterministic-random */
/*  Adams–Moulton Method implementation – version #7  */
#include <cstdio>
#include <cstdlib>

static unsigned deterministic_state = 123456789u;
static int deterministic_rand()
{
    deterministic_state = deterministic_state * 1103515245u + 12345u;
    return static_cast<int>((deterministic_state / 65536u) % 32768u);
}

int main()
{
    /* ---------- configuration (generated internally) ---------- */
    // Use deterministic pseudo‑random generator instead of time‑based seed
    // to produce a medium‑sized, diverse input that is repeatable.
    int maxSteps = 20;                                 /* stack capacity */
    int stepsCnt = 5 + deterministic_rand() % 11;      /* 5 … 15 steps */
    float step_h  = 0.1f + ( deterministic_rand() % 41 ) / 100.0f; /* 0.1 … 0.5 */
    float param_a = 0.5f + ( deterministic_rand() % 151 ) / 100.0f;/* 0.5 … 2.0 */
    float start_y = 1.0f + ( deterministic_rand() % 901 ) / 100.0f;/* 1.0 … 10.0 */

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
