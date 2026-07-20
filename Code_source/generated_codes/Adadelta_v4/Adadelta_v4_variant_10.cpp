/* LLM input variant 10: large-safe-stress */
#include <iostream>
#include <cmath>

// ------------------------------------------------------------
// Helper: compute square of a value (verbose)
// ------------------------------------------------------------
void squareValue(float src, float &dst)
{
    // dst = src * src
    dst = src * src;
}

// ------------------------------------------------------------
// Helper: update exponential moving average (verbose)
// ------------------------------------------------------------
void movingAverage(float oldAvg, float newSample, float decay, float &newAvg)
{
    // newAvg = decay * oldAvg + (1 - decay) * newSample
    float oneMinusDecay = 1.0f - decay;
    float termOld = decay * oldAvg;
    float termNew = oneMinusDecay * newSample;
    newAvg = termOld + termNew;
}

// ------------------------------------------------------------
// Helper: compute parameter delta using Adadelta formula (verbose)
// ------------------------------------------------------------
float computeDelta(float rmsPrevUpd, float rmsGrad, float grad)
{
    // delta = - (rmsPrevUpd / rmsGrad) * grad
    float ratio = rmsPrevUpd / rmsGrad;
    float delta = - ratio * grad;
    return delta;
}

// ------------------------------------------------------------
// Helper: print current state of all vectors (verbose)
// ------------------------------------------------------------
void printState(int step,
                const float *param,
                const float *grad,
                const float *avgSqGrad,
                const float *avgSqUpd,
                int dim)
{
    std::cout << "=== Step " << step << " ===============================\n";
    for (int i = 0; i < dim; ++i)
    {
        std::cout << "Param[" << i << "] = " << param[i]
                  << " | Grad[" << i << "] = " << grad[i]
                  << " | AvgSqGrad[" << i << "] = " << avgSqGrad[i]
                  << " | AvgSqUpd[" << i << "] = " << avgSqUpd[i] << "\n";
    }
    std::cout << "---------------------------------------------------\n";
}

// ------------------------------------------------------------
// Main driver: runs a larger Adadelta optimisation on synthetic data
// ------------------------------------------------------------
int main()
{
    // --------------------------------------------------------
    // Larger synthetic data (stack arrays, safe size)
    // --------------------------------------------------------
    const int DIM = 1000; // safe for typical stack limits
    float paramVec[DIM] = {0.0f};
    float gradVec[DIM];
    float avgSqGrad[DIM] = {0.0f};
    float avgSqUpd[DIM]  = {0.0f};

    // Initialise gradient vector with a deterministic pattern:
    // index % 5 == 0 -> 0.0
    // index % 5 == 1 -> tiny 1e-8
    // index % 5 == 2 -> huge 1e8
    // index % 5 == 3 -> negative huge -1e8
    // else -> sin(i) * 3.14159
    for (int i = 0; i < DIM; ++i)
    {
        switch (i % 5)
        {
            case 0: gradVec[i] = 0.0f; break;
            case 1: gradVec[i] = 1e-8f; break;
            case 2: gradVec[i] = 1e8f; break;
            case 3: gradVec[i] = -1e8f; break;
            default: gradVec[i] = std::sinf(static_cast<float>(i)) * 3.14159f; break;
        }
    }

    // Hyper‑parameters (float only)
    float rho = 0.95f;          // decay factor
    float eps = 1e-6f;          // epsilon for numerical stability
    int   totalIter = 5;        // a few more iterations for stress

    // --------------------------------------------------------
    // Verbose optimisation loop
    // --------------------------------------------------------
    int iter = 0;
    while (iter < totalIter)
    {
        // Print state before this iteration
        printState(iter, paramVec, gradVec, avgSqGrad, avgSqUpd, DIM);

        // Process each dimension separately (for‑loop with reversed index order)
        for (int idx = DIM - 1; idx >= 0; --idx)
        {
            // 1) Square current gradient
            float gradSq = 0.0f;
            squareValue(gradVec[idx], gradSq);

            // 2) Update running average of squared gradients
            float newAvgSqGrad = 0.0f;
            movingAverage(avgSqGrad[idx], gradSq, rho, newAvgSqGrad);
            avgSqGrad[idx] = newAvgSqGrad;               // store back

            // 3) Compute RMS (root‑mean‑square) of previous updates and current grad
            float rmsPrevUpd = 0.0f;
            float rmsGrad = 0.0f;

            float tmpPrev = avgSqUpd[idx] + eps;          // add epsilon first
            float tmpGrad = avgSqGrad[idx] + eps;         // add epsilon first

            rmsPrevUpd = sqrtf(tmpPrev);                 // sqrt of (prev avg + eps)
            rmsGrad    = sqrtf(tmpGrad);                  // sqrt of (grad avg + eps)

            // 4) Compute delta (parameter update)
            float delta = computeDelta(rmsPrevUpd, rmsGrad, gradVec[idx]);

            // 5) Square the delta for the second moving average
            float deltaSq = 0.0f;
            squareValue(delta, deltaSq);

            // 6) Update running average of squared updates
            float newAvgSqUpd = 0.0f;
            movingAverage(avgSqUpd[idx], deltaSq, rho, newAvgSqUpd);
            avgSqUpd[idx] = newAvgSqUpd;                 // store back

            // 7) Apply the delta to the parameter
            paramVec[idx] = paramVec[idx] + delta;
        }

        ++iter;
    }

    // Final state after all iterations
    printState(totalIter, paramVec, gradVec, avgSqGrad, avgSqUpd, DIM);

    // --------------------------------------------------------
    // Print a concise summary of the results
    // --------------------------------------------------------
    std::cout << "=== Final Parameters =============================\n";
    for (int i = 0; i < DIM; ++i)
    {
        std::cout << "param[" << i << "] = " << paramVec[i] << "\n";
    }
    std::cout << "===================================================\n";

    return 0;
}
