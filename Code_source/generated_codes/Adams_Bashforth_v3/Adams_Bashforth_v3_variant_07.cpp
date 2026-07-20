#include <iostream>
#include <cmath>   // only for reference, not for double usage

/* LLM input variant 7: reverse-adversarial */

// ------------------------------------------------------------
// Helper: right‑hand side of the ODE dy/dt = -y
// ------------------------------------------------------------
float rhs(float t_val, float y_val)
{
    // Simple linear decay; edge‑case: any t or y works
    return -y_val;
}

// ------------------------------------------------------------
// Helper: allocate a float array on the heap
// ------------------------------------------------------------
float* allocate_array(int length)
{
    // length must be positive; we do not check for brevity
    return new float[length];
}

// ------------------------------------------------------------
// Helper: deallocate a float array from the heap
// ------------------------------------------------------------
void free_array(float* ptr)
{
    delete[] ptr;
}

// ------------------------------------------------------------
// Helper: perform a 2‑step Adams‑Bashforth integration
// ------------------------------------------------------------
void adams_bashforth_2step(
    float stepSize,          // integration step h
    int   totalSteps,        // number of steps to take
    float initTime,          // t0
    float initValue)         // y0
{
    // Allocate storage for t and y on the heap
    float* t_series = allocate_array(totalSteps + 1);
    float* y_series = allocate_array(totalSteps + 1);
    float* f_series = allocate_array(totalSteps + 1); // stores f(t,y)

    // --------------------------------------------------------
    // Initialise first point (Euler step for the very first value)
    // --------------------------------------------------------
    t_series[0] = initTime;
    y_series[0] = initValue;
    f_series[0] = rhs(t_series[0], y_series[0]);

    // A single Euler step to obtain the second point (required by AB2)
    t_series[1] = t_series[0] + stepSize;
    // Reordered arithmetic: (f0 * stepSize) + y0
    y_series[1] = y_series[0] + (f_series[0] * stepSize);
    f_series[1] = rhs(t_series[1], y_series[1]);

    // --------------------------------------------------------
    // Verbose output of the initial two points
    // --------------------------------------------------------
    std::cout << "Step 0: t = " << t_series[0] << " , y = " << y_series[0] << "\n";
    std::cout << "Step 1: t = " << t_series[1] << " , y = " << y_series[1] << "\n";

    // --------------------------------------------------------
    // Main integration loop (while‑style, not a for‑loop)
    // --------------------------------------------------------
    int idx = 2;                           // current index
    while (idx <= totalSteps)
    {
        // Compute next time value
        t_series[idx] = t_series[idx - 1] + stepSize;

        // Adams‑Bashforth 2‑step formula (reordered for variety)
        // y_{n+1} = y_n + h * ( (3/2) * f_n - (1/2) * f_{n-1} )
        // Reorder: ((3*f_n - f_{n-1}) * 0.5f) * h + y_n
        float predictor = ((3.0f * f_series[idx - 1] - f_series[idx - 2]) * 0.5f) * stepSize;
        y_series[idx] = y_series[idx - 1] + predictor;

        // Evaluate RHS at the new point
        f_series[idx] = rhs(t_series[idx], y_series[idx]);

        // Verbose step‑by‑step printout
        std::cout << "Step " << idx << ": t = " << t_series[idx]
                  << " , y = " << y_series[idx] << "\n";

        // Advance index
        idx = idx + 1;    // explicit increment, not ++idx
    }

    // --------------------------------------------------------
    // Clean up heap memory
    // --------------------------------------------------------
    free_array(t_series);
    free_array(y_series);
    free_array(f_series);
}

// ------------------------------------------------------------
// Main entry point
// ------------------------------------------------------------
int main()
{
    // Reverse‑adversarial ordering of runs:
    // 1) Small realistic step, many steps
    // 2) Large step, fewer steps
    // 3) Zero step, minimal steps (edge case)

    float stepSize;
    int   totalSteps;
    float startTime = 0.0f;
    float startValue = 1.0f;

    // First run: small step, larger number of steps
    stepSize = 0.05f;          // very small step to stress precision
    totalSteps = 20;           // more steps to amplify workload
    std::cout << "=== Run with stepSize = 0.05 (reverse‑adversarial small step) ===\n";
    adams_bashforth_2step(stepSize, totalSteps, startTime, startValue);

    // Second run: relatively large step, moderate steps
    stepSize = 0.7f;           // large step, may cause noticeable error
    totalSteps = 12;
    std::cout << "\n=== Run with stepSize = 0.7 (reverse‑adversarial large step) ===\n";
    adams_bashforth_2step(stepSize, totalSteps, startTime, startValue);

    // Third run: zero step size, minimal steps (edge case)
    stepSize = 0.0f;           // zero step size (no progress)
    totalSteps = 5;
    std::cout << "\n=== Run with stepSize = 0 (reverse‑adversarial zero step) ===\n";
    adams_bashforth_2step(stepSize, totalSteps, startTime, startValue);

    return 0;
}
