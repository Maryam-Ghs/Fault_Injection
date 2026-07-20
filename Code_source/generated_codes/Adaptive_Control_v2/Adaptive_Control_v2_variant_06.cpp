#include <iostream>
using namespace std;

/* LLM input variant 6: ordered-structured */

/* -------------------------------------------------------------
   Global configuration for ordered-structured input
   ------------------------------------------------------------- */
constexpr int TOTAL_STEPS = 100;   // deterministic, larger, ordered size

/* -------------------------------------------------------------
   Helper: generate a symmetric, monotonic reference signal
   ------------------------------------------------------------- */
void generateReference(int idx, float &refSignal)
{
    int half = TOTAL_STEPS / 2;
    if (idx <= half)
        refSignal = static_cast<float>(idx) * 0.1f;               // increasing linearly
    else
        refSignal = static_cast<float>(TOTAL_STEPS - idx) * 0.1f; // decreasing symmetrically
}

/* -------------------------------------------------------------
   Helper: discrete‑time plant simulation (first‑order)
   y(k+1) = a*y(k) + b*u(k)
   ------------------------------------------------------------- */
void simulatePlant(float yPrev, float uPrev, float aPlant, float bPlant,
                   float &yNext)
{
    // explicit Euler with unit sampling time
    float termA = aPlant * yPrev;
    float termB = bPlant * uPrev;
    yNext = termA + termB;
}

/* -------------------------------------------------------------
   Helper: reference model simulation
   ym(k+1) = a_m*ym(k) + b_m*r(k)
   ------------------------------------------------------------- */
void simulateModel(float ymPrev, float rPrev, float aModel, float bModel,
                   float &ymNext)
{
    float termA = aModel * ymPrev;
    float termB = bModel * rPrev;
    ymNext = termA + termB;
}

/* -------------------------------------------------------------
   Helper: adaptive law (gradient update)
   θ(k+1) = θ(k) + γ * e(k) * φ(k)
   where φ = y(k)   (regressor)
   ------------------------------------------------------------- */
void updateParameter(float error, float phi, float gamma,
                     float thetaPrev, float &thetaNext)
{
    float adapt = gamma * error * phi;
    thetaNext = thetaPrev + adapt;
}

/* -------------------------------------------------------------
   Main entry point
   ------------------------------------------------------------- */
int main()
{
    // ---------------------------------------------------------
    // Simulation settings (ordered‑structured)
    // ---------------------------------------------------------
    const int totalSteps = TOTAL_STEPS; // number of samples
    float aPlant = 0.8f;                // stable plant dynamics
    float bPlant = 1.0f;                // moderate control gain
    float aModel = -0.5f;               // stable reference model
    float bModel = 1.0f;                // model input gain
    float gamma  = 0.5f;                // moderate adaptation gain

    // ---------------------------------------------------------
    // Stack‑allocated state vectors
    // ---------------------------------------------------------
    float plantState[TOTAL_STEPS];
    float modelState[TOTAL_STEPS];
    float controlSig[TOTAL_STEPS];
    float paramEst[TOTAL_STEPS];
    float reference[TOTAL_STEPS];
    float trackingError[TOTAL_STEPS];

    // ---------------------------------------------------------
    // Initialise all vectors to zero
    // ---------------------------------------------------------
    for (int i = 0; i < totalSteps; ++i)
    {
        plantState[i]      = 0.0f;
        modelState[i]      = 0.0f;
        controlSig[i]      = 0.0f;
        paramEst[i]        = 0.0f;
        reference[i]       = 0.0f;
        trackingError[i]   = 0.0f;
    }

    // ---------------------------------------------------------
    // Main adaptive control loop (while‑loop for structural variety)
    // ---------------------------------------------------------
    int k = 0;
    while (k < totalSteps - 1)
    {
        // ---- 1. Produce reference signal for current step ----
        generateReference(k, reference[k]);

        // ---- 2. Compute tracking error e(k) = y(k) - ym(k) ----
        float error = plantState[k] - modelState[k];
        trackingError[k] = error;   // store for later inspection

        // ---- 3. Adaptive law: update parameter estimate θ(k+1) ----
        //    Regressor φ(k) is the current plant output y(k)
        updateParameter(error, plantState[k], gamma,
                        paramEst[k], paramEst[k+1]);

        // ---- 4. Compute control input u(k) using current estimate ----
        //    u(k) = -θ(k) * y(k) + r(k)   (simple MRAC law)
        float termNeg = -paramEst[k] * plantState[k];
        float termRef = reference[k];
        controlSig[k] = termNeg + termRef;   // fused expression

        // ---- 5. Propagate plant state to next step ----
        simulatePlant(plantState[k], controlSig[k],
                      aPlant, bPlant, plantState[k+1]);

        // ---- 6. Propagate reference model state to next step ----
        simulateModel(modelState[k], reference[k],
                      aModel, bModel, modelState[k+1]);

        // ---- 7. Move to next sample ----
        ++k;
    }

    // ---------------------------------------------------------
    // Final step: compute last reference and error (k = totalSteps-1)
    // ---------------------------------------------------------
    generateReference(totalSteps-1, reference[totalSteps-1]);
    trackingError[totalSteps-1] = plantState[totalSteps-1] -
                                 modelState[totalSteps-1];

    // ---------------------------------------------------------
    // Print results (verbose)
    // ---------------------------------------------------------
    cout << "Step  Ref   yPlant   yModel   uCtrl   theta   Error\n";
    cout << "---------------------------------------------------\n";
    for (int i = 0; i < totalSteps; ++i)
    {
        cout << i << "   ";
        cout << reference[i] << "   ";
        cout << plantState[i] << "   ";
        cout << modelState[i] << "   ";
        cout << controlSig[i] << "   ";
        cout << paramEst[i] << "   ";
        cout << trackingError[i] << "\n";
    }

    return 0;
}
