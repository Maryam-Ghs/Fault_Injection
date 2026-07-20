#include <iostream>
#include <cmath>   // only for reference, not for double usage

/* LLM input variant 9: medium-deterministic-random */

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

// Simple deterministic pseudo‑random generator (LCG) with fixed seed
unsigned int lcg(unsigned int& state)
{
    state = state * 1664525u + 1013904223u;
    return state;
}

// Map LCG output to a float in [min, max)
float rand_float(unsigned int& state, float min, float max)
{
    unsigned int r = lcg(state);
    return min + (static_cast<float>(r) / static_cast<float>(UINT_MAX)) * (max - min);
}

// Map LCG output to an int in [min, max]
int rand_int(unsigned int& state, int min, int max)
{
    unsigned int r = lcg(state);
    return min + (r % (static_cast<unsigned int>(max - min + 1)));
}

// ------------------------------------------------------------
// Main entry point
// ------------------------------------------------------------
int main()
{
    // Deterministic seed ensures reproducibility
    unsigned int rng_state = 123456789u;

    // Generate three distinct runs with medium‑sized, pseudo‑random parameters
    for (int run = 1; run <= 3; ++run)
    {
        // Step size in a reasonable range (0.05 … 1.05)
        float stepSize = rand_float(rng_state, 0.05f, 1.05f);

        // Total steps between 15 and 45 for a medium workload
        int totalSteps = rand_int(rng_state, 15, 45);

        // Start time fixed at 0 for simplicity
        float startTime = 0.0f;

        // Initial value in a diverse range (0.5 … 2.5)
        float startValue = rand_float(rng_state, 0.5f, 2.5f);

        std::cout << "\n=== Run " << run << " : stepSize = " << stepSize
                  << ", totalSteps = " << totalSteps
                  << ", startValue = " << startValue << " ===\n";

        adams_bashforth_2step(stepSize, totalSteps, startTime, startValue);
    }

    return 0;
}
