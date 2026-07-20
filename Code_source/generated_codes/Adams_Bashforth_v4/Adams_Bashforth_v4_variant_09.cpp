#include <iostream>
#include <cmath>   // for expf

/* LLM input variant 9: medium-deterministic-random */

int main()
{
    /* --------------------------------------------------------------
       Verbose, step‑by‑step implementation of the 4‑step Adams‑Bashforth
       explicit multistep method.  All data lives on the stack, the whole
       program is inside main(), and only float / int types are used.
       -------------------------------------------------------------- */

    /* ---- 1.  Define the ODE to be solved:  dy/dt = f(t,y) = -y ---- */
    /* No const allowed – everything is mutable. */
    float (*f)(float, float) = nullptr;                     // function pointer
    f = [](float t_val, float y_val) -> float               // lambda expression
    {
        /* The right‑hand side is simply -y .  The parameter t_val is
           unused but kept to illustrate the generic signature.          */
        float minus_one = -1.0f;
        float result   = minus_one * y_val;                 // expanded op
        return result;
    };

    /* ---- 2.  Edge‑case heavy input parameters ----------------------- */
    // Deterministic pseudo‑random‑looking parameters
    float start_t   = 0.0f;          // initial time
    float final_t   = 12.91f;        // end time (non‑integral, medium size)
    float step_h    = 0.37f;         // step size (pseudo‑random decimal)
    int   max_step  = 1000;          // safety cap for array size (still within stack)

    /* Edge case: zero step size – the algorithm will just repeat the
       initial condition without advancing.  No division by step_h is
       performed, so this is safe.                                      */
    if (step_h == 0.0f)
    {
        std::cout << "Step size is zero – the solution will not evolve.\n";
    }

    /* ---- 3.  Allocate stack arrays for t and y ---------------------- */
    float t_series[2000];
    float y_series[2000];

    /* ---- 4.  Initial condition -------------------------------------- */
    float cur_t = start_t;
    float cur_y = 0.987f;               // y(0) = 0.987 (deterministic variant)

    t_series[0] = cur_t;
    y_series[0] = cur_y;

    /* ---- 5.  Compute the first three steps with a simple Euler method
              to seed the multistep scheme. --------------------------- */
    int   seed_cnt = 0;               // how many seed points we have
    float temp_f0, temp_f1, temp_f2, temp_f3; // placeholders for f values

    while (seed_cnt < 3 && cur_t < final_t)
    {
        /* f at current point */
        float f_now = f(cur_t, cur_y);

        /* Euler update: y_new = y_old + h * f_now  (expanded) */
        float h_mul_f = step_h * f_now;
        float y_new   = cur_y + h_mul_f;

        /* Advance time */
        float t_new = cur_t + step_h;

        /* Store results */
        ++seed_cnt;
        t_series[seed_cnt] = t_new;
        y_series[seed_cnt] = y_new;

        /* Prepare for next iteration */
        cur_t = t_new;
        cur_y = y_new;
    }

    /* ---- 6.  Load the first four f‑values needed for Adams‑Bashforth ---- */
    temp_f0 = f(t_series[0], y_series[0]);   // f_{n-3}
    temp_f1 = f(t_series[1], y_series[1]);   // f_{n-2}
    temp_f2 = f(t_series[2], y_series[2]);   // f_{n-1}
    temp_f3 = f(t_series[3], y_series[3]);   // f_n   (if we have it)

    /* If we stopped early because final_t was reached, fill missing slots */
    while (seed_cnt < 3)
    {
        ++seed_cnt;
        t_series[seed_cnt] = cur_t;   // repeat last time
        y_series[seed_cnt] = cur_y;   // repeat last y
    }

    /* ---- 7.  Main Adams‑Bashforth loop (while‑loop version) ---------- */
    int step_index = 3;   // we already have points 0,1,2,3

    while (t_series[step_index] < final_t && step_index < max_step-1)
    {
        /* ---- a)  Evaluate f at the newest point (already have temp_f3) */
        /* ---- b)  Compute the Adams‑Bashforth increment                */
        /*          y_{n+1} = y_n + h/24 * (55 f_n - 59 f_{n-1}
                                         + 37 f_{n-2} - 9 f_{n-3})   */
        float term1 = 55.0f * temp_f3;
        float term2 = -59.0f * temp_f2;
        float term3 = 37.0f * temp_f1;
        float term4 = -9.0f * temp_f0;

        float sum_terms = term1 + term2 + term3 + term4;   // expanded addition
        float coeff      = step_h / 24.0f;                // h/24
        float increment  = coeff * sum_terms;            // h/24 * sum

        /* ---- c)  Update y ------------------------------------------------ */
        float y_next = cur_y + increment;                // expanded op

        /* ---- d)  Advance time -------------------------------------------- */
        float t_next = cur_t + step_h;

        /* ---- e)  Store new point ------------------------------------------ */
        ++step_index;
        t_series[step_index] = t_next;
        y_series[step_index] = y_next;

        /* ---- f)  Shift the f‑history for the next iteration --------------- */
        temp_f0 = temp_f1;   // f_{n-2} becomes f_{n-3}
        temp_f1 = temp_f2;   // f_{n-1} becomes f_{n-2}
        temp_f2 = temp_f3;   // f_n     becomes f_{n-1}
        temp_f3 = f(t_next, y_next); // new f_{n+1}

        /* ---- g)  Update current variables --------------------------------- */
        cur_t = t_next;
        cur_y = y_next;
    }

    /* ---- 8.  Print the results ---------------------------------------- */
    std::cout << "Adams‑Bashforth 4‑step solution (float arithmetic)\n";
    std::cout << "Step size h = " << step_h << "\n";
    std::cout << "t\t\t y (numeric)\t y (exact)\t error\n";

    int print_idx = 0;
    while (print_idx <= step_index)
    {
        float t_val   = t_series[print_idx];
        float y_num   = y_series[print_idx];
        /* Exact solution of dy/dt = -y with y(0)=1 is exp(-t) */
        // Adjusted exact solution for our initial y(0)=0.987
        float y_exact = 0.987f * expf(-t_val);
        float diff    = y_num - y_exact;
        float err_abs = (diff >= 0.0f) ? diff : -diff;   // absolute error

        std::cout << t_val << "\t " << y_num << "\t " << y_exact
                  << "\t " << err_abs << "\n";

        ++print_idx;
    }

    return 0;
}
