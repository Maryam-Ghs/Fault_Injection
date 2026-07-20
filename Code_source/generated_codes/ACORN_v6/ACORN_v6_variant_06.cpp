#include <iostream>
#include <vector>

/* LLM input variant 6: ordered-structured */

int main()
{
    // --------------------------------------------------------------
    // 1. Define small predefined data sets (order of the generator
    //    and the initial seeds).  All data lives inside main().
    // --------------------------------------------------------------
    std::vector<int> rngOrder;          // order (k) for each ACORN instance
    std::vector<int> seedBase;          // initial seed for each instance

    // Structured ordered arrays – ascending order and symmetric seeds.
    for (int i = 2; i <= 11; ++i)               // orders 2..11 (sorted)
    {
        rngOrder.push_back(i);
    }

    // Symmetric seed pattern: 1000, 2000, ..., 10000 (sorted)
    for (int i = 1; i <= 10; ++i)
    {
        seedBase.push_back(i * 1000);
    }

    // --------------------------------------------------------------
    // 2. Parameters for the simulation
    // --------------------------------------------------------------
    int stepCount = 10;                  // how many random numbers to produce

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
