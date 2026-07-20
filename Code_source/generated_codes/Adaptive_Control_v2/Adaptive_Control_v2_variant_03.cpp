#include <iostream>
using namespace std;

/* LLM input variant 3: zeros-and-ones */

/* -------------------------------------------------------------
   Helper: generate a piece‑wise reference signal (zero/one emphasis)
   ------------------------------------------------------------- */
void generateReference(int idx, float &refSignal)
{
    // Emphasize zeros and ones
    if (idx % 2 == 0)   refSignal = 0.0f;   // even steps -> zero
    else                refSignal = 1.0f;   // odd steps  -> one
}

/* -------------------------------------------------------------
   Helper: discrete‑time plant simulation (first‑order)
   y(k+1) = a*y(k) + b*u(k)
   ------------------------------------------------------------- */
void simulatePlant(float yPrev, float uPrev, float aPlant, float bPlant,
                   float &yNext)
{
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
    // Simulation settings (zero/one emphasis)
    // ---------------------------------------------------------
    int totalSteps = 10;               // reduced steps for brevity
    float aPlant = 1.0f;               // identity dynamics
    float bPlant = 0.0f;               // no control influence on plant
    float aModel = 1.0f;               // identity model dynamics
    float bModel = 0.0f;               // model ignores reference input
    float gamma  = 1.0f;               // neutral adaptation gain

    // ---------------------------------------------------------
    // Stack‑allocated state vectors
    // ---------------------------------------------------------
    float plantState[50];
    float modelState[50];
    float controlSig[50];
    float paramEst[50];
    float reference[50];
    float trackingError[50];

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
        trackingError[k] = error;

        // ---- 3. Adaptive law: update parameter estimate θ(k+1) ----
        updateParameter(error, plantState[k], gamma,
                        paramEst[k], paramEst[k+1]);

        // ---- 4. Compute control input u(k) using current estimate ----
        float termNeg = -paramEst[k] * plantState[k];
        float termRef = reference[k];
        controlSig[k] = termNeg + termRef;

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
