#include <iostream>
#include <iomanip>
#include <cstdlib>

/* LLM input variant 9: medium-deterministic-random */
int main()
{
    /*--- generate a deterministic pseudo‑random linear ODE  dy/dt = A*y + B*t + C  ---*/
    std::srand(123456789);                     // fixed seed for reproducibility
    float A = static_cast<float>(std::rand() % 9 - 4);   // -4 … 4
    float B = static_cast<float>(std::rand() % 9 - 4);   // -4 … 4
    float C = static_cast<float>(std::rand() % 9 - 4);   // -4 … 4

    /*--- problem data ---------------------------------------------------*/
    float h = 0.07f;               // step size (different from original)
    int  N = 30;                  // number of steps (larger than original)
    float t[31];                  // time values (stack array, size N+1)
    float y[31];                  // solution values (stack array, size N+1)

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
        float y_guess = y[idx] + h * f_now;                  // ŷ_{n+1}

        /* corrector: Adams–Moulton */
        float f_next  = rhs(t[idx + 1], y_guess);            // f(t_{n+1}, ŷ_{n+1})
        float half_h  = h * 0.5f;                            // h/2
        float sum_f   = f_now + f_next;                     // (f_n + f_{n+1})
        y[idx + 1]    = y[idx] + half_h * sum_f;            // y_{n+1}

        ++idx;
    }

    /*--- output ----------------------------------------------------------*/
    std::cout << "Random ODE: dy/dt = " << A << "*y + " << B << "*t + " << C << "\n\n";
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
