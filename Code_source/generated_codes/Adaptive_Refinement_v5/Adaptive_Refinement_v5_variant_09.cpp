#include <iostream>
#include <cstdlib>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

int main()
{
    // ------------------------------------------------------------
    // Version #5 – Adaptive Refinement (all logic inside main)
    // ------------------------------------------------------------

    // ---- Step 1 : define problem size and refinement parameters ----
    int initialSize          = 1200;   // number of points in the coarse array (variant)
    int maximumArrayCapacity = 5000;   // stack allocation size – must hold refined data
    int refinementPasses     = 3;      // how many times we refine the data (variant)
    float errorThreshold     = 0.15f;  // if neighbour difference exceeds this, we refine (variant)

    // ---- Step 2 : allocate two working buffers on the stack ----------
    float rawArray[5000];
    float refinedArray[5000];

    // ---- Step 3 : deterministic pseudo‑random generator and fill the initial data --
    // Simple linear congruential generator with fixed seed
    unsigned int lcg = 123456789u;
    const unsigned int LCG_A = 1103515245u;
    const unsigned int LCG_C = 12345u;
    const unsigned int LCG_M = 0x7fffffffu; // 2^31‑1

    for (int i = 0; i < initialSize; ++i)
    {
        lcg = (LCG_A * lcg + LCG_C) & LCG_M;
        float randomVal = static_cast<float>(lcg) / static_cast<float>(LCG_M);
        rawArray[i] = randomVal;
    }

    // ---- Step 4 : initialise bookkeeping variables -----------------
    int lengthCurrent = initialSize;   // how many valid entries are in rawArray
    int lengthNext    = 0;             // will hold length after a refinement pass
    int passCounter   = 0;            // number of performed passes

    // ---- Step 5 : start the adaptive refinement loop ---------------
    while (passCounter < refinementPasses)
    {
        // reset the length for the next buffer
        lengthNext = 0;

        // walk through the current data, examining each neighbour pair
        int index = 0;
        while (index < (lengthCurrent - 1))
        {
            // fetch the two adjacent values
            float leftValue  = rawArray[index];
            float rightValue = rawArray[index + 1];

            // compute absolute difference without using fabs (to stay in float)
            float diffTemp1   = leftValue - rightValue;
            float diffAbs1    = diffTemp1;
            if (diffAbs1 < 0.0f) diffAbs1 = -diffAbs1;   // manual absolute value

            // decide whether refinement is required
            if (diffAbs1 > errorThreshold)
            {
                // we need a midpoint – compute it in two separate steps
                float sumValues   = leftValue + rightValue;
                float halfFactor  = 0.5f;
                float middlePoint = sumValues * halfFactor;

                // write left value and the newly created midpoint into the next buffer
                refinedArray[lengthNext] = leftValue;
                ++lengthNext;
                refinedArray[lengthNext] = middlePoint;
                ++lengthNext;
            }
            else
            {
                // no refinement – only copy the left value
                refinedArray[lengthNext] = leftValue;
                ++lengthNext;
            }

            // advance to the next pair (skip one position)
            ++index;
        }

        // copy the final element of the current buffer (rightmost point)
        refinedArray[lengthNext] = rawArray[lengthCurrent - 1];
        ++lengthNext;

        // ---- Step 6 : prepare for the next iteration -----------------
        // swap the role of the two buffers by copying references (no std::swap to keep it simple)
        int copyIdx = 0;
        while (copyIdx < lengthNext)
        {
            rawArray[copyIdx] = refinedArray[copyIdx];
            ++copyIdx;
        }

        // update length information
        lengthCurrent = lengthNext;

        // increment pass counter
        ++passCounter;
    }

    // ---- Step 7 : output the results --------------------------------
    std::cout << "Adaptive refinement completed.\n";
    std::cout << "Final number of points : " << lengthCurrent << "\n";

    // compute and display the sum of all values (as a simple checksum)
    float totalSum = 0.0f;
    int sumIdx = 0;
    while (sumIdx < lengthCurrent)
    {
        totalSum = totalSum + rawArray[sumIdx];
        ++sumIdx;
    }
    std::cout << "Sum of all point values : " << totalSum << "\n";

    // print first ten values to give a glimpse of the refined data
    std::cout << "First ten values (if available):\n";
    int printLimit = 10;
    if (lengthCurrent < printLimit) printLimit = lengthCurrent;
    int outIdx = 0;
    while (outIdx < printLimit)
    {
        std::cout << "  [" << outIdx << "] = " << rawArray[outIdx] << "\n";
        ++outIdx;
    }

    return 0;
}
