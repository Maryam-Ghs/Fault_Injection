// LLM input variant 9: medium-deterministic-random
// Adadelta optimizer implementation - version #6
// -------------------------------------------------
// This program demonstrates a step‑by‑step Adadelta update
// on a tiny set of parameters using only float and int types.
// All memory is allocated on the heap (new/delete) and the
// input data are deliberately chosen to stress edge cases
// (zeros, very large magnitudes, tiny values).
// -------------------------------------------------

#include <iostream>
#include <cmath>    // for sqrtf
#include <cstdio>   // for printf

// -----------------------------------------------------------------
// Helper: allocate a float array on the heap
float* createArray(int length)
{
    // length must be positive, but we keep it simple
    float* ptr = new float[length];
    return ptr;
}

// Helper: release a heap‑allocated array
void destroyArray(float* ptr)
{
    delete[] ptr;
}

// Helper: fill an array with deterministic pseudo‑random‑looking values
void fillEdgeCases(float* arr, int length)
{
    // Simple linear congruential generator with fixed seed
    unsigned int seed = 123456789u;
    const unsigned int a = 1103515245u;
    const unsigned int c = 12345u;
    const unsigned int m = 0x7fffffffu; // 2^31-1

    for (int i = 0; i < length; ++i)
    {
        // Introduce explicit zeros at regular intervals
        if (i % 13 == 0)
        {
            arr[i] = 0.0f;
            continue;
        }

        seed = (a * seed + c) & m;
        // Map to range [-10000.0, 10000.0] with a step of 0.1
        int raw = static_cast<int>(seed % 200001u); // 0 .. 200000
        float val = (static_cast<float>(raw) - 100000.0f) / 10.0f;
        arr[i] = val;
    }
}

// Helper: zero‑initialize an accumulator array
void zeroArray(float* arr, int length)
{
    for (int i = 0; i < length; ++i)
        arr[i] = 0.0f;
}

// Helper: display an array with a label
void showArray(char* label, float* arr, int length)
{
    std::printf("%s:", label);
    for (int i = 0; i < length; ++i)
        std::printf(" %f", arr[i]);
    std::printf("\n");
}

// Core: perform a single Adadelta update over all parameters
void adadeltaUpdate(
    float* param,       // current parameters (weights)
    float* grad,        // current gradients
    float* accGrad,     // accumulated squared gradients (E[g^2])
    float* accDelta,    // accumulated squared updates (E[Δx^2])
    int    size,
    float  rho,
    float  eps)
{
    // Loop over each dimension
    for (int idx = 0; idx < size; ++idx)
    {
        // 1) Update running average of squared gradients
        //    Eg_new = rho * Eg_old + (1 - rho) * grad^2
        float gradSq = grad[idx] * grad[idx];
        accGrad[idx] = rho * accGrad[idx] + (1.0f - rho) * gradSq;

        // 2) Compute RMS of updates and gradients (add epsilon first)
        //    RMS_Δ = sqrt(Edx + eps)
        //    RMS_g = sqrt(Eg + eps)
        float rmsDelta = std::sqrt(accDelta[idx] + eps);
        float rmsGrad  = std::sqrt(accGrad[idx] + eps);

        // 3) Determine the update value
        //    Δ = (RMS_Δ / RMS_g) * grad
        float update = (rmsDelta / rmsGrad) * grad[idx];

        // 4) Apply the update to the parameter (note the minus sign)
        param[idx] = param[idx] - update;

        // 5) Update running average of squared updates
        //    Edx_new = rho * Edx_old + (1 - rho) * Δ^2
        float updSq = update * update;
        accDelta[idx] = rho * accDelta[idx] + (1.0f - rho) * updSq;
    }
}

// -----------------------------------------------------------------
// Main driver
int main()
{
    // -------------------------------------------------------------
    // Configuration (edge‑case heavy)
    // -------------------------------------------------------------
    int    dim      = 11;         // number of parameters (odd size, larger than original)
    float  rho      = 0.85f;     // decay factor (slightly lower)
    float  eps      = 1e-5f;     // epsilon to avoid division by zero

    // -------------------------------------------------------------
    // Allocate all required arrays on the heap
    // -------------------------------------------------------------
    float* weights   = createArray(dim);
    float* grads     = createArray(dim);
    float* eg        = createArray(dim);   // E[g^2]
    float* edelta    = createArray(dim);   // E[Δx^2]

    // -------------------------------------------------------------
    // Initialise arrays
    // -------------------------------------------------------------
    // We start with a known set of weights (deterministic pseudo‑random)
    fillEdgeCases(weights, dim);
    // Gradients are also deterministic pseudo‑random (mirroring weight pattern)
    fillEdgeCases(grads, dim);
    // Accumulators start at zero
    zeroArray(eg, dim);
    zeroArray(edelta, dim);

    // Show initial state
    showArray((char*)"Initial weights", weights, dim);
    showArray((char*)"Initial grads  ", grads, dim);
    showArray((char*)"E[g^2] init   ", eg, dim);
    showArray((char*)"E[Δx^2] init  ", edelta, dim);
    std::printf("\n");

    // -------------------------------------------------------------
    // Perform several Adadelta steps (using a while loop to vary style)
    // -------------------------------------------------------------
    int step = 0;
    int maxSteps = 4;   // increased number of steps for richer behavior
    while (step < maxSteps)
    {
        std::printf("=== Step %d ===\n", step + 1);
        adadeltaUpdate(weights, grads, eg, edelta, dim, rho, eps);

        // Print results after this step
        showArray((char*)"Weights after update", weights, dim);
        showArray((char*)"E[g^2] after update", eg, dim);
        showArray((char*)"E[Δx^2] after update", edelta, dim);
        std::printf("\n");

        // For the next iteration we deliberately change the gradients
        // to another deterministic pattern (multiply every third entry by -1)
        for (int i = 0; i < dim; ++i)
        {
            if (i % 3 == 0)
                grads[i] = -grads[i];
            // else keep as is
        }

        ++step;
    }

    // -------------------------------------------------------------
    // Clean up heap memory
    // -------------------------------------------------------------
    destroyArray(weights);
    destroyArray(grads);
    destroyArray(eg);
    destroyArray(edelta);

    return 0;
}
