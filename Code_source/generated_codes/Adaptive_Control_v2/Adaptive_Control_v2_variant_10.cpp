#include <iostream>
#include <vector>
using namespace std;

/* LLM input variant 10: large-safe-stress */

/* -------------------------------------------------------------
   Helper: generate a piece‑wise reference signal (edge‑case heavy)
   ------------------------------------------------------------- */
void generateReference(int idx, float &refSignal)
{
    // Extended pattern for larger horizon
    if (idx % 100 < 20)               refSignal = 0.0f;          // silence
    else if (idx % 100 < 40)          refSignal = -7.5f;         // negative step
    else if (idx % 100 < 60)          refSignal = 15.0f;         // large positive step
    else if (idx % 100 < 80)          refSignal = 0.0f;          // back to zero
    else                              refSignal = -12.5f;        // large negative step
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
    // Simulation settings (larger but safe)
    // ---------------------------------------------------------
    const int totalSteps = 2000;          // increased horizon
    const float aPlant = 0.9f;          // near‑unit plant dynamics
    const float bPlant = 20.0f;          // larger control gain
    const float aModel = -0.4f;          // stable reference model
    const float bModel = 1.0f;           // model input gain
    const float gamma  = 10.0f;          // more aggressive adaptation

    // ---------------------------------------------------------
    // Dynamically allocated state vectors (heap‑based)
    // ---------------------------------------------------------
    vector<float> plantState(totalSteps, 0.0f);
    vector<float> modelState(totalSteps, 0.0f);
    vector<float> controlSig(totalSteps, 0.0f);
    vector<float> paramEst(totalSteps, 0.0f);
    vector<float> reference(totalSteps, 0.0f);
    vector<float> trackingError(totalSteps, 0.0f);

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

        ++k;
    }

    // ---------------------------------------------------------
    // Final step: compute last reference and error (k = totalSteps-1)
    // ---------------------------------------------------------
    generateReference(totalSteps-1, reference[totalSteps-1]);
    trackingError[totalSteps-1] = plantState[totalSteps-1] -
                                 modelState[totalSteps-1];

    // ---------------------------------------------------------
    // Print results (first 100 steps for brevity)
    // ---------------------------------------------------------
    const int printLimit = 100;
    cout << "Step  Ref   yPlant   yModel   uCtrl   theta   Error\n";
    cout << "---------------------------------------------------\n";
    for (int i = 0; i < totalSteps && i < printLimit; ++i)
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
