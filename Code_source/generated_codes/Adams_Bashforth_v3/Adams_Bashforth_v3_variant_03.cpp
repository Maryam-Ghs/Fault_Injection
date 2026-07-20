#include <iostream>
#include <cmath>   // only for reference, not for double usage

/* LLM input variant 3: zeros-and-ones */

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
    // First run: zero step size, zero start value
    float stepSize = 0.0f;          // zero step size (no progress)
    int   totalSteps = 1;           // minimal steps to keep AB2 valid
    float startTime = 0.0f;
    float startValue = 0.0f;

    std::cout << "=== Run with stepSize = 0, startValue = 0 ===\n";
    adams_bashforth_2step(stepSize, totalSteps, startTime, startValue);

    // Second run: unit step size, unit start value, single step
    stepSize = 1.0f;                // identity step size
    totalSteps = 1;
    startValue = 1.0f;

    std::cout << "\n=== Run with stepSize = 1, startValue = 1 (single step) ===\n";
    adams_bashforth_2step(stepSize, totalSteps, startTime, startValue);

    // Third run: unit step size, unit start value, two steps
    stepSize = 1.0f;
    totalSteps = 2;
    startValue = 1.0f;

    std::cout << "\n=== Run with stepSize = 1, startValue = 1 (two steps) ===\n";
    adams_bashforth_2step(stepSize, totalSteps, startTime, startValue);

    return 0;
}
