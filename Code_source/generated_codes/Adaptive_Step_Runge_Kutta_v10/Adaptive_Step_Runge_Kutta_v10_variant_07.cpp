/* LLM input variant 7: reverse-adversarial */
/*  Adaptive Runge–Kutta implementation – version #10
 *
 *  Constraints:
 *  - Only float and int are used (no double, long, unsigned, const)
 *  - All code resides inside int main()
 *  - Input is generated internally as large random arrays
 *  - Results are printed to stdout
 *  - Verbose step‑by‑step comments, fused arithmetic expressions,
 *    stack‑allocated arrays, and a deliberately different structure.
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>

int main()
{
    /* --------------------------------------------------------------
     *  1.  Problem definition
     *      dy/dt = f(t,y)  with  f(t,y) = -y        (exponential decay)
     *      Integrate from tStart to tStop for many independent
     *      initial values stored in an array.
     * -------------------------------------------------------------- */
    int nSamples = 1000;                 // size of the deterministic input array
    float tStart   = 0.0f;               // integration start
    float tStop    = 5.0f;               // integration end
    float initArr[1000];                 // stack array for initial y values
    float resultArr[1000];               // stack array for final y values

    /* --------------------------------------------------------------
     *  2.  Generate deterministic, reverse‑ordered initial conditions
     *      descending from 1.0 to 0.0 (worst‑case for decay magnitude)
     * -------------------------------------------------------------- */
    for (int idx = 0; idx < nSamples; ++idx)
    {
        initArr[idx] = 1.0f - static_cast<float>(idx) / (nSamples - 1);
    }

    /* --------------------------------------------------------------
     *  3.  Adaptive Runge–Kutta integration (step‑doubling)
     *      - Single RK4 step with size h    → yBig
     *      - Two RK4 half‑steps with size h/2 → yFine
     *      - Error estimate = |yBig - yFine|
     *      - Accept if error ≤ tol, otherwise shrink h
     * -------------------------------------------------------------- */
    float tolerance = 1e-4f;             // error tolerance for all trajectories

    for (int idx = 0; idx < nSamples; ++idx)
    {
        /* ----- 3.1  Initialise per‑trajectory variables ----- */
        float timeVar   = tStart;                // current time
        float stateVar  = initArr[idx];          // current solution y(t)
        float stepSize  = 0.05f;                 // initial step size (smaller for more steps)

        /* ----- 3.2  Main integration loop ----- */
        while (timeVar < tStop)
        {
            /* Ensure the last step lands exactly on tStop */
            if (timeVar + stepSize > tStop)
                stepSize = tStop - timeVar;

            /* ----- 3.2.1  One full RK4 step (size = stepSize) ----- */
            float k1 = -stateVar;
            float k2 = -(stateVar + 0.5f * stepSize * k1);
            float k3 = -(stateVar + 0.5f * stepSize * k2);
            float k4 = -(stateVar + stepSize * k3);
            float yBig = stateVar + stepSize *
                         (k1 + 2.0f * k2 + 2.0f * k3 + k4) / 6.0f;

            /* ----- 3.2.2  Two half‑steps (size = stepSize/2) ----- */
            float halfStep = 0.5f * stepSize;
            float yFine    = stateVar;           // start of the two half‑steps
            for (int sub = 0; sub < 2; ++sub)
            {
                float kh1 = -yFine;
                float kh2 = -(yFine + 0.5f * halfStep * kh1);
                float kh3 = -(yFine + 0.5f * halfStep * kh2);
                float kh4 = -(yFine + halfStep * kh3);
                yFine = yFine + halfStep *
                        (kh1 + 2.0f * kh2 + 2.0f * kh3 + kh4) / 6.0f;
            }

            /* ----- 3.2.3  Error estimate and step‑size control ----- */
            float errEst = fabsf(yBig - yFine);

            if (errEst <= tolerance)               // accept the step
            {
                timeVar  = timeVar + stepSize;     // advance time
                stateVar = yFine;                  // keep the more accurate value
                stepSize = stepSize * 1.5f;        // try a larger step next time
            }
            else                                    // reject, shrink step
            {
                stepSize = stepSize * 0.5f;
                /* No time or state update – repeat with smaller step */
            }
        }

        /* ----- 3.3  Store final value for this trajectory ----- */
        resultArr[idx] = stateVar;
    }

    /* --------------------------------------------------------------
     *  4.  Output – print the first ten results for verification
     * -------------------------------------------------------------- */
    std::printf("Adaptive RK results (first 10 of %d samples):\n", nSamples);
    for (int i = 0; i < 10; ++i)
    {
        std::printf("  y(%g) ≈ %g   (initial y0 = %g)\n",
                    static_cast<double>(tStop),
                    static_cast<double>(resultArr[i]),
                    static_cast<double>(initArr[i]));
    }

    return 0;
}
