#include <iostream>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */

int main()
{
    /*--- deterministic linear ODE  dy/dt = A*y + B*t + C  ---*/
    float A = 0.0f;   // coefficient for y (duplicate value)
    float B = 0.0f;   // coefficient for t (duplicate value)
    float C = 0.0f;   // constant term (duplicate value)

    /*--- problem data ---------------------------------------------------*/
    float h = 0.1f;               // step size
    int  N = 20;                  // number of steps
    float t[21];                  // time values (stack array)
    float y[21];                  // solution values (stack array)

    /*--- initial condition ----------------------------------------------*/
    t[0] = 0.0f;
    y[0] = 1.0f;                  // y(0) = 1

    /*--- lambda for the RHS of the ODE ----------------------------------*/
    auto rhs = [&](float tt, float yy) -> float
    {
        return A * yy + B * tt + C;
    };

    /*--- Adams–Moulton 2‑step (predictor‑corrector) ---------------------*/
    int idx = 0;
    while (idx < N)
    {
        /* advance time */
        t[idx + 1] = t[idx] + h;

        /* predictor: explicit Euler */
        float f_now   = rhs(t[idx], y[idx]);                 // f(t_n , y_n)
        float y_guess = y[idx] + h * f_now;                  // ŷ_{n+1}

        /* corrector: Adams–Moulton */
        float f_next  = rhs(t[idx + 1], y_guess);           // f(t_{n+1}, ŷ_{n+1})
        float half_h  = h * 0.5f;                            // h/2
        float sum_f   = f_now + f_next;                     // (f_n + f_{n+1})
        y[idx + 1]    = y[idx] + half_h * sum_f;            // y_{n+1}

        ++idx;
    }

    /*--- output ----------------------------------------------------------*/
    std::cout << "Deterministic ODE: dy/dt = " << A << "*y + " << B << "*t + " << C << "\n\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  t       y\n";
    std::cout << "----------------\n";
    int j = 0;
    while (j <= N)
    {
        std::cout << std::setw(6) << t[j] << "  " << std::setw(8) << y[j] << "\n";
        ++j;
    }
    return 0;
}
