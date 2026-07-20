#include <iostream>
#include <cmath>
#include <cstdio>

/* LLM input variant 5: duplicate-heavy */

int main()
{
    // -------------------- Version #10 --------------------
    // Adaptive Simpson's rule using only float and int.
    // All data are stored in heap arrays, everything lives inside main().

    // -----------------------------------------------------------------
    // Test vector 1 : integrate f(x) = x*x on [0 , 1]   (exact = 1/3)
    // Test vector 2 : integrate g(x) = sinf(x) on [0 , pi] (exact = 2)
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
    // First test vector (x*x) with duplicate intervals
    // -----------------------------------------------------------------
    auto func = [](float x)->float { return x * x; };

    const int dupCount1 = 5; // number of identical intervals to push
    sp = 0;
    for (int i = 0; i < dupCount1; ++i)
    {
        low[sp]   = 0.0f;
        high[sp]  = 1.0f;
        fLow[sp]  = func(low[sp]);
        fHigh[sp] = func(high[sp]);
        float half = (high[sp] - low[sp]) * 0.5f;
        float mid  = low[sp] + half;
        fMid[sp]  = func(mid);
        sPrev[sp] = (high[sp] - low[sp]) * (fLow[sp] + 4.0f * fMid[sp] + fHigh[sp]) / 6.0f;
        ++sp;                                      // push count = dupCount1
    }

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

    std::printf("Integral of x*x from 0 to 1  = %.7f   (error = %.7f)\n",
                result, fabsf(result - (1.0f/3.0f)));

    // -----------------------------------------------------------------
    // Second test vector (sinf) with duplicate intervals
    // -----------------------------------------------------------------
    auto func2 = [](float x)->float { return sinf(x); };

    const int dupCount2 = 4; // number of identical intervals to push
    sp = 0;
    for (int i = 0; i < dupCount2; ++i)
    {
        low[sp]   = 0.0f;
        high[sp]  = 3.1415926535f;                 // pi approximated as float
        fLow[sp]  = func2(low[sp]);
        fHigh[sp] = func2(high[sp]);
        float half = (high[sp] - low[sp]) * 0.5f;
        float mid  = low[sp] + half;
        fMid[sp]  = func2(mid);
        sPrev[sp] = (high[sp] - low[sp]) * (fLow[sp] + 4.0f * fMid[sp] + fHigh[sp]) / 6.0f;
        ++sp;                                      // push count = dupCount2
    }

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

    std::printf("Integral of sin(x) from 0 to pi = %.7f   (error = %.7f)\n",
                result, fabsf(result - 2.0f));

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
