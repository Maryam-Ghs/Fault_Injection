/* LLM input variant 9: medium-deterministic-random */
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdint>

int main()
{
    /* --------------------------------------------------------------
     *  1.  Problem definition
     *      dy/dt = f(t,y)  with  f(t,y) = -y        (exponential decay)
     *      Integrate from tStart to tStop for many independent
     *      initial values stored in an array.
     * -------------------------------------------------------------- */
    int nSamples = 750;                  // size of the deterministic input array
    float tStart   = 0.0f;               // integration start
    float tStop    = 5.0f;               // integration end
    float initArr[750];                  // stack array for initial y values
    float resultArr[750];               // stack array for final y values

    /* --------------------------------------------------------------
     *  2.  Generate deterministic pseudo‑random initial conditions
     *      in the interval [-0.5, 1.5)
     * -------------------------------------------------------------- */
    uint32_t seed = 0xF0E1D2C3u;         // fixed seed for reproducibility
    const uint32_t a = 1664525u;
    const uint32_t c = 1013904223u;
    for (int idx = 0; idx < nSamples; ++idx)
    {
        seed = a * seed + c;            // LCG step (mod 2^32 automatically)
        // Map to [-0.5, 1.5) by taking lower 12 bits (0‑4095) → divide by 2000, shift
        initArr[idx] = (static_cast<float>(seed & 0xFFFu) / 2000.0f) - 0.5f;
    }

    /* --------------------------------------------------------------
     *  3.  Adaptive Runge–Kutta integration (step‑doubling)
     *      - Single RK4 step with size h    → yBig
     *      - Two RK4 half‑steps with size h/2 → yFine
     *      - Error estimate = |yBig - yFine|
     *      - Accept if error ≤ tol, otherwise shrink h
     * -------------------------------------------------------------- */
    float tolerance = 5e-5f;            // tighter error tolerance for all trajectories

    for (int idx = 0; idx < nSamples; ++idx)
    {
        /* ----- 3.1  Initialise per‑trajectory variables ----- */
        float timeVar   = tStart;                // current time
        float stateVar  = initArr[idx];          // current solution y(t)
        float stepSize  = 0.08f;                 // initial step size (smaller than original)

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
                stepSize = stepSize * 1.6f;        // try a larger step next time
            }
            else                                    // reject, shrink step
            {
                stepSize = stepSize * 0.45f;
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
