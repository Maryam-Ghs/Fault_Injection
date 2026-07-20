#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 5: duplicate-heavy */
int main() {
    // -------------------- configuration --------------------
    int sampleCnt = 120;                 // length of the simulation
    float learnRate = 0.02f;             // adaptation gain
    float plantA = 0.7f;                 // plant pole
    float plantB = 0.2f;                 // plant zero

    // -------------------- allocate memory --------------------
    float* arrRef   = new float[sampleCnt];   // reference signal
    float* arrOut   = new float[sampleCnt];   // plant output
    float* arrCtrl  = new float[sampleCnt];   // control effort
    float* arrTheta = new float[sampleCnt];   // adaptive gain estimate

    // -------------------- generate duplicate-heavy reference --------------------
    for (int i = 0; i < sampleCnt; ++i) {
        // Repeating pattern: 0.5, -0.5, 0.5, -0.5, ...
        arrRef[i] = (i % 2 == 0) ? 0.5f : -0.5f;
    }

    // -------------------- initial conditions --------------------
    arrOut[0]   = 0.0f;
    arrCtrl[0]  = 0.0f;
    arrTheta[0] = 0.0f;

    // -------------------- simulation loop (while) --------------------
    int idx = 0;
    while (idx < sampleCnt - 1) {
        // ---------- error computation ----------
        float refVal   = arrRef[idx];
        float outVal   = arrOut[idx];
        float err      = refVal - outVal;          // tracking error

        // ---------- regressor (simple) ----------
        float phi      = outVal;                  // using plant output as regressor

        // ---------- parameter update ----------
        float thetaPrev = arrTheta[idx];
        float adaptTerm = learnRate * err * phi;
        float thetaNew  = thetaPrev + adaptTerm;
        arrTheta[idx + 1] = thetaNew;

        // ---------- control law ----------
        float ctrlVal  = thetaNew * refVal;
        arrCtrl[idx]   = ctrlVal;

        // ---------- plant dynamics ----------
        float aTerm    = plantA * outVal;
        float bTerm    = plantB * ctrlVal;
        float outNext  = aTerm + bTerm;
        arrOut[idx + 1] = outNext;

        // ---------- move to next sample ----------
        ++idx;
    }

    // -------------------- output results --------------------
    std::cout << "Step\tRef\tOut\tCtrl\tTheta\n";
    for (int i = 0; i < sampleCnt; ++i) {
        std::cout << i << '\t'
                  << arrRef[i]   << '\t'
                  << arrOut[i]   << '\t'
                  << arrCtrl[i]  << '\t'
                  << arrTheta[i] << '\n';
    }

    // -------------------- clean up --------------------
    delete[] arrRef;
    delete[] arrOut;
    delete[] arrCtrl;
    delete[] arrTheta;

    return 0;
}
