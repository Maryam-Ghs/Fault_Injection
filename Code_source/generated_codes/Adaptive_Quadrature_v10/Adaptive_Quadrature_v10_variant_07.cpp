#include <iostream>
#include <cmath>
#include <cstdio>

/* LLM input variant 7: reverse-adversarial */
int main()
{
    // -------------------- Version #10 --------------------
    // Adaptive Simpson's rule using only float and int.
    // All data are stored in heap arrays, everything lives inside main().

    // -----------------------------------------------------------------
    // Test vector 1 : integrate f(x) = sin(100*x) on [π , 0]   (exact = 0)
    // Test vector 2 : integrate g(x) = 1/(x^2+1e-6) on [1 , 0] (exact ≈ 1000.0005)
    // -----------------------------------------------------------------

    const int maxDepth = 2000;                 // increased stack size for deeper recursion
    float *low   = new float[maxDepth];        // left end of interval
    float *high  = new float[maxDepth];        // right end of interval
    float *fLow  = new float[maxDepth];        // f(low)
    float *fHigh = new float[maxDepth];        // f(high)
    float *fMid  = new float[maxDepth];        // f(mid)
    float *sPrev = new float[maxDepth];        // Simpson estimate for the interval

    float eps = 1e-8f;                         // tighter tolerance to force many splits
    float result;                              // final integral value
    int   sp;                                  // stack pointer (top)

    // -----------------------------------------------------------------
    // First test vector (sin(100*x) reversed interval)
    // -----------------------------------------------------------------
    auto func = [](float x)->float { return sinf(100.0f * x); };

    // initialise the stack with the whole interval [π,0] (reversed)
    sp = 0;
    low[sp]   = 3.1415926535f;                 // pi as high end, now low
    high[sp]  = 0.0f;                          // zero as high end, now high
    fLow[sp]  = func(low[sp]);
    fHigh[sp] = func(high[sp]);
    float half1 = (high[sp] - low[sp]) * 0.5f;
    float mid1  = low[sp] + half1;
    fMid[sp]  = func(mid1);
    sPrev[sp] = (high[sp] - low[sp]) * (fLow[sp] + 4.0f * fMid[sp] + fHigh[sp]) / 6.0f;
    ++sp;                                      // push count = 1

    result = 0.0f;

    // main adaptive loop (manual stack)
    while (sp > 0)
    {
        // pop the top interval
        --sp;
        float a   = low[sp];
        float b   = high[sp];
        float fa  = fLow[sp];
        float fb  = fHigh[sp];
        float fm  = fMid[sp];
        float s0  = sPrev[sp];

        // split the interval
        float half = (b - a) * 0.5f;
        float m    = a + half;
        float leftMid  = a + half * 0.5f;        // (a+m)/2
        float rightMid = m + half * 0.5f;       // (m+b)/2

        float fLeftMid  = func(leftMid);
        float fRightMid = func(rightMid);

        // Simpson estimates for the two halves
        float sL = (m - a) * (fa + 4.0f * fLeftMid + fm) / 6.0f;
        float sR = (b - m) * (fm + 4.0f * fRightMid + fb) / 6.0f;
        float sTot = sL + sR;

        // error test (standard 15*eps criterion)
        if (fabsf(sTot - s0) < 15.0f * eps)
        {
            result += sTot;
        }
        else
        {
            // push right half
            low[sp]   = m;
            high[sp]  = b;
            fLow[sp]  = fm;
            fHigh[sp] = fb;
            fMid[sp]  = fRightMid;
            sPrev[sp] = sR;
            ++sp;

            // push left half
            low[sp]   = a;
            high[sp]  = m;
            fLow[sp]  = fa;
            fHigh[sp] = fm;
            fMid[sp]  = fLeftMid;
            sPrev[sp] = sL;
            ++sp;
        }
    }

    std::printf("Integral of sin(100*x) from pi to 0 = %.7f   (error = %.7f)\n",
                result, fabsf(result - 0.0f));

    // -----------------------------------------------------------------
    // Second test vector (1/(x^2+1e-6) reversed interval)
    // -----------------------------------------------------------------
    auto func2 = [](float x)->float { return 1.0f / (x * x + 0.000001f); };

    // re‑initialise the stack for the interval [1, 0] (reversed)
    sp = 0;
    low[sp]   = 1.0f;
    high[sp]  = 0.0f;
    fLow[sp]  = func2(low[sp]);
    fHigh[sp] = func2(high[sp]);
    float half2 = (high[sp] - low[sp]) * 0.5f;
    float mid2  = low[sp] + half2;
    fMid[sp]  = func2(mid2);
    sPrev[sp] = (high[sp] - low[sp]) * (fLow[sp] + 4.0f * fMid[sp] + fHigh[sp]) / 6.0f;
    ++sp;                                      // push count = 1

    result = 0.0f;

    // adaptive loop again
    while (sp > 0)
    {
        --sp;
        float a   = low[sp];
        float b   = high[sp];
        float fa  = fLow[sp];
        float fb  = fHigh[sp];
        float fm  = fMid[sp];
        float s0  = sPrev[sp];

        float half = (b - a) * 0.5f;
        float m    = a + half;
        float leftMid  = a + half * 0.5f;
        float rightMid = m + half * 0.5f;

        float fLeftMid  = func2(leftMid);
        float fRightMid = func2(rightMid);

        float sL = (m - a) * (fa + 4.0f * fLeftMid + fm) / 6.0f;
        float sR = (b - m) * (fm + 4.0f * fRightMid + fb) / 6.0f;
        float sTot = sL + sR;

        if (fabsf(sTot - s0) < 15.0f * eps)
        {
            result += sTot;
        }
        else
        {
            low[sp]   = m;
            high[sp]  = b;
            fLow[sp]  = fm;
            fHigh[sp] = fb;
            fMid[sp]  = fRightMid;
            sPrev[sp] = sR;
            ++sp;

            low[sp]   = a;
            high[sp]  = m;
            fLow[sp]  = fa;
            fHigh[sp] = fm;
            fMid[sp]  = fLeftMid;
            sPrev[sp] = sL;
            ++sp;
        }
    }

    // Approximate exact integral analytically: ∫_0^1 1/(x^2+1e-6) dx = (1/√1e-6) * atan(1/√1e-6)
    // For display we keep the original error computation against a pre‑computed value.
    const float exact2 = (1.0f / sqrtf(0.000001f)) * atanf(1.0f / sqrtf(0.000001f));
    std::printf("Integral of 1/(x^2+1e-6) from 1 to 0 = %.7f   (error = %.7f)\n",
                result, fabsf(result - (-exact2)));

    // ---------------------------------------------------------------
    // clean up heap memory
    delete [] low;
    delete [] high;
    delete [] fLow;
    delete [] fHigh;
    delete [] fMid;
    delete [] sPrev;

    return 0;
}
