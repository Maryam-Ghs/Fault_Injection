// Adams–Bashforth Method – version #8
/* LLM input variant 2: small-diverse */
// ------------------------------------------------------------
// Implements the 4‑step explicit Adams‑Bashforth scheme.
// Uses only float and int, no const, no double, no unsigned.
// ------------------------------------------------------------

#include <iostream>
#include <vector>

// ------------------------------------------------------------
// right‑hand side of the ODE: dy/dt = -y + t
// ------------------------------------------------------------
float rhs(float t_val, float y_val)
{
    // reordered arithmetic: (t_val) - (y_val)
    return t_val - y_val;
}

// ------------------------------------------------------------
// generate the first three points with a simple Euler step
// ------------------------------------------------------------
void seed_with_euler(std::vector<float>& time_vec,
                     std::vector<float>& sol_vec,
                     float step_sz,
                     int seed_cnt)
{
    int i = 0;
    while (i < seed_cnt)
    {
        float t_now = time_vec[i];
        float y_now = sol_vec[i];

        // Euler: y_{i+1} = y_i + h * f(t_i, y_i)
        float inc = step_sz * rhs(t_now, y_now);
        float y_next = y_now + inc;          // reordered: y_now + inc
        float t_next = t_now + step_sz;      // t_i + h

        time_vec.push_back(t_next);
        sol_vec.push_back(y_next);
        ++i;
    }
}

// ------------------------------------------------------------
// 4‑step Adams‑Bashforth iteration
// ------------------------------------------------------------
void adams_bashforth(std::vector<float>& time_vec,
                     std::vector<float>& sol_vec,
                     float step_sz,
                     int total_steps)
{
    // need at least 4 points to start
    int cur = 3; // index of the latest known point
    while (cur < total_steps)
    {
        // fetch the four most recent f values
        float f_n   = rhs(time_vec[cur],     sol_vec[cur]);
        float f_nm1 = rhs(time_vec[cur - 1], sol_vec[cur - 1]);
        float f_nm2 = rhs(time_vec[cur - 2], sol_vec[cur - 2]);
        float f_nm3 = rhs(time_vec[cur - 3], sol_vec[cur - 3]);

        // combine with reordered arithmetic
        float combo = 55.0f * f_n;
        combo -= 59.0f * f_nm1;
        combo += 37.0f * f_nm2;
        combo -= 9.0f  * f_nm3;

        // y_{n+1} = y_n + (h/24) * combo
        float delta = step_sz * combo / 24.0f; // reordered: (h * combo) / 24
        float y_next = sol_vec[cur] + delta;  // y_n + delta
        float t_next = time_vec[cur] + step_sz;

        time_vec.push_back(t_next);
        sol_vec.push_back(y_next);
        ++cur;
    }
}

// ------------------------------------------------------------
// driver
// ------------------------------------------------------------
int main()
{
    // deterministic test vector parameters
    float step_sz   = 0.2f;   // step size
    int   max_steps = 8;    // total number of points (including seed)
    int   seed_cnt  = 3;    // points produced by Euler before AB starts

    // containers for time and solution
    std::vector<float> time_vec;
    std::vector<float> sol_vec;

    // initial condition y(0) = 0.7
    time_vec.push_back(0.0f);
    sol_vec.push_back(0.7f);

    // generate seed points
    seed_with_euler(time_vec, sol_vec, step_sz, seed_cnt);

    // apply Adams‑Bashforth to reach the desired length
    adams_bashforth(time_vec, sol_vec, step_sz, max_steps);

    // output the results
    int idx = 0;
    while (idx < (int)time_vec.size())
    {
        std::cout << "t = " << time_vec[idx]
                  << " , y = " << sol_vec[idx] << '\n';
        ++idx;
    }
    return 0;
}
