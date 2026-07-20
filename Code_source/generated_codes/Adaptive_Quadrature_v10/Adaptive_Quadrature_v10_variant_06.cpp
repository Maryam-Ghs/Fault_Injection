#include <iostream>
#include <cmath>
#include <cstdio>

/* LLM input variant 6: ordered-structured */

int main()
{
    // -------------------- Version #10 --------------------
    // Adaptive Simpson's rule using only float and int.
    // All data are stored in heap arrays, everything lives inside main().

    // -----------------------------------------------------------------
    // Test vector 1 : integrate f(x) = x^3 on [-1 , 1]   (exact = 0)
    // Test vector 2 : integrate h(x) = cosf(x) on [-pi , pi] (exact = 0)
    // -----------------------------------------------------------------

    const int maxDepth = 1000;                 // maximum stack size
    float *low   = new float[maxDepth];        // left end of interval
    float *high  = new float[maxDepth];        // right end of interval
    float *fLow  = new float[maxDepth];        // f(low)
    float *fHigh = new float[maxDepth];        // f(high)
    float *fMid  = new float[maxDepth];        // f(mid)
    float *sPrev = new float[maxDepth];        // Simpson estimate for the interval

    float eps = 1e-5f;                         // tolerance
    float result;                              // final integral value
    int   sp;                                  // stack pointer (top)

    // -----------------------------------------------------------------
    // First test vector (x^3)
    // -----------------------------------------------------------------
    auto func = [](float x)->float { return x * x * x; };

    // initialise the stack with the whole interval [-1,1]
    sp = 0;
    low[sp]   = -1.0f;
    high[sp]  =  1.0f;
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

    std::printf("Integral of x^3 from -1 to 1  = %.7f   (error = %.7f)\n",
                result, fabsf(result - 0.0f));

    // -----------------------------------------------------------------
    // Second test vector (cosf)
    // -----------------------------------------------------------------
    auto func2 = [](float x)->float { return cosf(x); };

    // re‑initialise the stack for the interval [-pi, pi]
    sp = 0;
    low[sp]   = -3.1415926535f;               // -pi approximated as float
    high[sp]  =  3.1415926535f;               //  pi approximated as float
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

    std::printf("Integral of cos(x) from -pi to pi = %.7f   (error = %.7f)\n",
                result, fabsf(result - 0.0f));

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
