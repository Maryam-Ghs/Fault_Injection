#include <iostream>
#include <cmath>
#include <cstdio>

/* LLM input variant 1: minimal-boundary */

int main()
{
    // -------------------- Version #10 --------------------
    // Adaptive Simpson's rule using only float and int.
    // All data are stored in heap arrays, everything lives inside main().

    // -----------------------------------------------------------------
    // Test vector 1 : integrate f(x) = x*x on [0 , 1]   (exact = 1/3)
    // Test vector 2 : integrate g(x) = sinf(x) on [0 , pi] (exact = 2)
    // -----------------------------------------------------------------

    const int maxDepth = 1;                    // minimal stack size
    float *low   = new float[maxDepth];
    float *high  = new float[maxDepth];
    float *fLow  = new float[maxDepth];
    float *fHigh = new float[maxDepth];
    float *fMid  = new float[maxDepth];
    float *sPrev = new float[maxDepth];

    float eps = 1e-2f;                         // relaxed tolerance for minimal input
    float result;
    int   sp;

    // -----------------------------------------------------------------
    // First test vector (x*x)
    // -----------------------------------------------------------------
    auto func = [](float x)->float { return x * x; };

    sp = 0;
    low[sp]   = 0.0f;
    high[sp]  = 1.0f;
    fLow[sp]  = func(low[sp]);
    fHigh[sp] = func(high[sp]);
    float half1 = (high[sp] - low[sp]) * 0.5f;
    float mid1  = low[sp] + half1;
    fMid[sp]  = func(mid1);
    sPrev[sp] = (high[sp] - low[sp]) * (fLow[sp] + 4.0f * fMid[sp] + fHigh[sp]) / 6.0f;
    ++sp;                                      // push count = 1

    result = 0.0f;

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

        float fLeftMid  = func(leftMid);
        float fRightMid = func(rightMid);

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

    std::printf("Integral of x*x from 0 to 1  = %.7f   (error = %.7f)\n",
                result, fabsf(result - (1.0f/3.0f)));

    // -----------------------------------------------------------------
    // Second test vector (sinf)
    // -----------------------------------------------------------------
    auto func2 = [](float x)->float { return sinf(x); };

    sp = 0;
    low[sp]   = 0.0f;
    high[sp]  = 3.1415926535f;
    fLow[sp]  = func2(low[sp]);
    fHigh[sp] = func2(high[sp]);
    float half2 = (high[sp] - low[sp]) * 0.5f;
    float mid2  = low[sp] + half2;
    fMid[sp]  = func2(mid2);
    sPrev[sp] = (high[sp] - low[sp]) * (fLow[sp] + 4.0f * fMid[sp] + fHigh[sp]) / 6.0f;
    ++sp;

    result = 0.0f;

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

    delete [] low;
    delete [] high;
    delete [] fLow;
    delete [] fHigh;
    delete [] fMid;
    delete [] sPrev;

    return 0;
}
