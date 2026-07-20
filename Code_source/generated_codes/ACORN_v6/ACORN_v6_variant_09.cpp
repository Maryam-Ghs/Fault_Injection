#include <iostream>
#include <vector>

/* LLM input variant 9: medium-deterministic-random */

int main()
{
    // --------------------------------------------------------------
    // 1. Define medium predefined data sets (order of the generator
    //    and the initial seeds).  All data lives inside main().
    // --------------------------------------------------------------
    std::vector<int> rngOrder;          // order (k) for each ACORN instance
    std::vector<int> seedBase;          // initial seed for each instance

    // Medium predefined arrays – deterministic but larger and more varied.
    rngOrder.push_back(5);   // first generator uses order 5
    rngOrder.push_back(6);   // second generator uses order 6
    rngOrder.push_back(7);   // third generator uses order 7
    rngOrder.push_back(8);   // fourth generator uses order 8
    rngOrder.push_back(9);   // fifth generator uses order 9

    seedBase.push_back(13579);   // seed for first generator
    seedBase.push_back(24680);   // seed for second generator
    seedBase.push_back(112233);  // seed for third generator
    seedBase.push_back(445566);  // seed for fourth generator
    seedBase.push_back(778899);  // seed for fifth generator

    // --------------------------------------------------------------
    // 2. Parameters for the simulation
    // --------------------------------------------------------------
    int stepCount = 30;                  // how many random numbers to produce per instance

    // --------------------------------------------------------------
    // 3. Iterate over every ACORN instance
    // --------------------------------------------------------------
    int instanceIdx = 0;
    while (instanceIdx < static_cast<int>(rngOrder.size()))
    {
        // ----------------------------------------------------------
        // 3a. Extract the order and seed for the current instance
        // ----------------------------------------------------------
        int currentOrder = rngOrder[instanceIdx];
        int currentSeed  = seedBase[instanceIdx];

        // ----------------------------------------------------------
        // 3b. Allocate the state vector (size = order) and initialise
        // ----------------------------------------------------------
        std::vector<int> rngState(currentOrder, 0);
        rngState[0] = currentSeed;       // only the first element gets the seed

        // ----------------------------------------------------------
        // 3c. Container for the generated values (as floats)
        // ----------------------------------------------------------
        std::vector<float> outSeries;
        outSeries.reserve(stepCount);

        // ----------------------------------------------------------
        // 3d. Produce 'stepCount' pseudo‑random numbers
        // ----------------------------------------------------------
        int stepIdx = 0;
        while (stepIdx < stepCount)
        {
            // ---- increment the first component ---------------------------------
            int inc = 1;
            int newFirst = rngState[0] + inc;
            rngState[0] = newFirst;

            // ---- propagate the addition through the remaining components ------------
            int innerIdx = 1;
            while (innerIdx < currentOrder)
            {
                // Reordered arithmetic: fetch previous value first
                int prevVal = rngState[innerIdx - 1];
                int curVal  = rngState[innerIdx];
                int updated = curVal + prevVal;   // curVal + prevVal  (order does not matter)
                rngState[innerIdx] = updated;
                ++innerIdx;
            }

            // ---- the output is the last component of the state -----------------------
            int rawOut = rngState[currentOrder - 1];

            // ---- normalise to the interval (0,1) using a float division -------------
            //    2^31‑1 = 2147483647 fits in a 32‑bit signed int
            float normOut = static_cast<float>(rawOut) / 2147483647.0f;

            // ---- store the result ----------------------------------------------------
            outSeries.push_back(normOut);
            ++stepIdx;
        }

        // ----------------------------------------------------------
        // 4. Print the generated series for the current instance
        // ----------------------------------------------------------
        std::cout << "Instance " << instanceIdx + 1
                  << " (order = " << rngOrder[instanceIdx]
                  << ", seed = " << seedBase[instanceIdx] << "):\n";

        int printIdx = 0;
        while (printIdx < static_cast<int>(outSeries.size()))
        {
            std::cout << "  [" << printIdx << "] = " << outSeries[printIdx] << '\n';
            ++printIdx;
        }
        std::cout << std::endl;

        ++instanceIdx;
    }

    // --------------------------------------------------------------
    // End of program
    // --------------------------------------------------------------
    return 0;
}
