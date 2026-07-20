#include <iostream>
#include <cmath>
#include <algorithm>

/* LLM input variant 8: sparse-skewed */

#define STEPS 20          // total number of points (including the start)
#define H 0.1f            // fixed step size

// derivative of the test ODE: dy/dt = -y + sin(t)
float rhs(float t, float y)
{
    return -y + sinf(t);
}

// one explicit Euler step – used to bootstrap the multistep method
void euler_one(float &t, float &y, float h)
{
    float k = rhs(t, y);
    y = y + h * k;
    t = t + h;
}

// Adams‑Bashforth 4‑step update (explicit formula)
void adams_bashforth4(
    float t_vals[STEPS],
    float y_vals[STEPS],
    float f_vals[STEPS],
    int start_idx,
    int end_idx,
    float h)
{
    int cur = start_idx;
    while (cur < end_idx)
    {
        // reuse previously stored derivatives
        float term0 = 55.0f * f_vals[cur];
        float term1 = -59.0f * f_vals[cur - 1];
        float term2 = 37.0f * f_vals[cur - 2];
        float term3 = -9.0f * f_vals[cur - 3];

        // compute next y value
        y_vals[cur + 1] = y_vals[cur] + (h / 24.0f) * (term0 + term1 + term2 + term3);

        // advance time
        t_vals[cur + 1] = t_vals[cur] + h;

        // store new derivative for the next iteration
        f_vals[cur + 1] = rhs(t_vals[cur + 1], y_vals[cur + 1]);

        ++cur;
    }
}

// simple driver – creates data, runs the method and prints the table
int main()
{
    // stack‑allocated arrays, initialized to zero (sparse rest)
    float t_seq[STEPS];
    float y_seq[STEPS];
    float f_seq[STEPS];
    std::fill_n(t_seq, STEPS, 0.0f);
    std::fill_n(y_seq, STEPS, 0.0f);
    std::fill_n(f_seq, STEPS, 0.0f);

    // initial condition
    t_seq[0] = 0.0f;
    y_seq[0] = 1.0f;
    f_seq[0] = rhs(t_seq[0], y_seq[0]);

    // generate the first three points with Euler (bootstrap)
    int idx = 0;
    while (idx < 3)
    {
        euler_one(t_seq[idx], y_seq[idx], H);
        ++idx;
        // after the Euler call, t_seq[idx] and y_seq[idx] hold the new values
        f_seq[idx] = rhs(t_seq[idx], y_seq[idx]);
    }

    // apply the 4‑step Adams‑Bashforth scheme only on a limited range
    // to keep later entries sparse (mostly zero)
    const int limited_end = 7; // compute up to index 7 (produces 8 populated points)
    adams_bashforth4(t_seq, y_seq, f_seq, 3, limited_end, H);

    // output the results
    std::cout << "  t\t   y\n";
    std::cout << "-------------------\n";
    for (int i = 0; i < STEPS; ++i)
    {
        std::cout << t_seq[i] << "\t" << y_seq[i] << "\n";
    }

    return 0;
}
