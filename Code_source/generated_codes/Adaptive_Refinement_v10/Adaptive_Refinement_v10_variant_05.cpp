/* LLM input variant 5: duplicate-heavy */
// Adaptive Refinement – version #10
// ------------------------------------------------------------
// This program demonstrates a simple adaptive interval refinement
// algorithm.  It works entirely with `float` and `int` types, uses
// heap‑allocated arrays, and follows a verbose, step‑by‑step style.
// ------------------------------------------------------------

#include <iostream>
#include <cmath>    // only for std::abs (float version)

//////////////////////////////////////////////////////////////
// Helper: absolute value for float (avoids double usage)
float fabsf(float v) {
    // Reordered arithmetic: multiply by 1.0f (no effect)
    return v * 1.0f < 0.0f ? -v : v;
}

// Simple scalar function to be refined.
// Example: f(x) = x*x - 2
float evaluate(float x) {
    // Reorder multiplication: x * x == x * x (no change)
    float sq = x * x;
    // Reorder subtraction: (sq - 2.0f) == -(2.0f - sq)
    return sq - 2.0f;
}

// ----------------------------------------------------------
// Class based implementation of adaptive refinement.
// ----------------------------------------------------------
class AdaptiveRefiner {
public:
    // Public members (no const, no unsigned, no double)
    int    maxDepth;          // maximum allowed refinement depth
    float  tolerance;        // error tolerance for stopping
    int    initialSegCount;  // number of initial coarse segments

    // Constructor – all members set later manually
    AdaptiveRefiner() {
        // Default values (will be overwritten in main)
        maxDepth          = 0;
        tolerance         = 0.0f;
        initialSegCount   = 0;
    }

    // Main routine: perform refinement and print results
    void run() {
        // --------------------------------------------------
        // 1. Allocate initial interval array on the heap.
        // --------------------------------------------------
        // Each interval is represented by its left and right bounds.
        // We allocate `initialSegCount` intervals plus one extra slot
        // for safety during splitting.
        int    totalSlots = initialSegCount + 10; // extra buffer
        float* lefts      = new float[totalSlots];
        float* rights     = new float[totalSlots];

        // --------------------------------------------------
        // 2. Fill the initial coarse grid (edge‑case heavy input).
        // --------------------------------------------------
        // Example: interval from -10 to +10 split uniformly.
        // Edge case: if initialSegCount is zero we still create one
        // interval covering the whole domain.
        int    segs = initialSegCount;
        if (segs < 1) segs = 1;                 // avoid division by zero
        // Duplicate‑heavy domain: both ends identical creates zero‑length intervals.
        float  domainL = 5.0f;
        float  domainR = 5.0f;
        // Step size computed with reordered arithmetic: (domainR - domainL) / segs
        float step = (domainR - domainL) / (float)segs;

        // Populate the arrays
        int idx = 0;
        while (idx < segs) {
            // left bound = domainL + idx * step
            lefts[idx]  = domainL + (float)idx * step;
            // right bound = left + step (reordered as step + left)
            rights[idx] = step + lefts[idx];
            idx = idx + 1;
        }

        // Keep track of the current number of intervals
        int currentCount = segs;

        // --------------------------------------------------
        // 3. Adaptive refinement loop.
        // --------------------------------------------------
        int depth = 0;
        while (depth < maxDepth) {
            // Flag to detect if any interval was split this pass
            int anySplit = 0;

            // Iterate over a snapshot of the current intervals.
            // We use a while‑loop with manual index increment (changed loop style).
            int i = 0;
            while (i < currentCount) {
                // Compute midpoint with reordered addition: (l + r) == (r + l)
                float mid = (rights[i] + lefts[i]) * 0.5f;

                // Function values at ends and midpoint
                float fL   = evaluate(lefts[i]);
                float fR   = evaluate(rights[i]);
                float fM   = evaluate(mid);

                // Linear interpolation estimate at midpoint:
                //   f_est = fL + (fR - fL) * (mid - left) / (right - left)
                // Reorder arithmetic for readability.
                float deltaX = rights[i] - lefts[i];
                float ratio  = (mid - lefts[i]) / deltaX;
                float f_est  = fL + (fR - fL) * ratio;

                // Error estimate (absolute difference)
                float err = fabsf(f_est - fM);

                // If error exceeds tolerance, split interval.
                if (err > tolerance) {
                    // --------------------------------------------------
                    // 3a. Insert new interval after the current one.
                    // --------------------------------------------------
                    // Ensure we have space – reallocate if needed.
                    if (currentCount + 1 >= totalSlots) {
                        // Double the buffer size (reordered multiplication)
                        int    newSize = totalSlots * 2;
                        float* newL    = new float[newSize];
                        float* newR    = new float[newSize];

                        // Copy existing data (verbose copy)
                        int j = 0;
                        while (j < currentCount) {
                            newL[j] = lefts[j];
                            newR[j] = rights[j];
                            j = j + 1;
                        }

                        // Release old buffers
                        delete[] lefts;
                        delete[] rights;

                        lefts      = newL;
                        rights     = newR;
                        totalSlots = newSize;
                    }

                    // Shift elements rightward to make room for new interval
                    int k = currentCount;
                    while (k > i + 1) {
                        lefts[k]  = lefts[k - 1];
                        rights[k] = rights[k - 1];
                        k = k - 1;
                    }

                    // First half: [left, mid]
                    rights[i] = mid;               // right bound becomes midpoint

                    // Second half: [mid, right]
                    lefts[i + 1]  = mid;           // left bound of new interval
                    // right bound already stored in rights[i+1] (unchanged)

                    // Update counters
                    currentCount = currentCount + 1;
                    anySplit     = anySplit + 1;

                    // Skip the newly created interval for this pass
                    i = i + 2;
                } else {
                    // No split – move to next interval
                    i = i + 1;
                }
            } // end while over intervals

            // If nothing was split, we are done early.
            if (anySplit == 0) break;

            // Increment depth counter (reordered addition)
            depth = depth + 1;
        } // end while depth

        // --------------------------------------------------
        // 4. Print final refined intervals.
        // --------------------------------------------------
        std::cout << "Adaptive refinement completed.\n";
        std::cout << "Depth reached: " << depth << "\n";
        std::cout << "Number of intervals: " << currentCount << "\n";
        std::cout << "Intervals (left , right):\n";

        // Verbose printing loop
        int p = 0;
        while (p < currentCount) {
            std::cout << "  [" << lefts[p] << " , " << rights[p] << "]\n";
            p = p + 1;
        }

        // --------------------------------------------------
        // 5. Clean up heap memory.
        // --------------------------------------------------
        delete[] lefts;
        delete[] rights;
    }
};

int main() {
    // --------------------------------------------------
    // Create an instance and configure edge‑case parameters.
    // --------------------------------------------------
    AdaptiveRefiner refiner;

    // Duplicate‑heavy configuration:
    //   - Zero tolerance forces maximal splitting up to maxDepth.
    //   - Small initial segment count combined with zero‑length domain.
    refiner.maxDepth        = 4;          // limit to avoid infinite splitting
    refiner.tolerance       = 0.0f;       // force splitting at every step
    refiner.initialSegCount = 6;          // start with several zero‑length intervals

    // Run the adaptive refinement process.
    refiner.run();

    return 0;
}
