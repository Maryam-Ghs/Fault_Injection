/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    // Seed random generator (not used for deterministic data)
    std::srand(static_cast<unsigned>(std::time(0)));

    // Size of the arrays (medium)
    int elemCount = 23;                     // a medium‑sized random array
    int *vecA = new int[elemCount];
    int *vecB = new int[elemCount];

    // Deterministic sparse and skewed data:
    // vecA has a few large clustered values, rest are zeros.
    // vecB has a different cluster of values, also mostly zeros.
    for (int i = 0; i < elemCount; ++i) {
        if (i >= 2 && i <= 4) {            // cluster in A at positions 2-4
            vecA[i] = 80 + i;              // values 82,83,84
        } else if (i >= 15 && i <= 16) {   // another small cluster in A
            vecA[i] = 95;                  // values 95,95
        } else {
            vecA[i] = 0;                    // sparse zeros elsewhere
        }

        if (i >= 0 && i <= 1) {            // cluster in B at positions 0-1
            vecB[i] = 70 + i * 5;          // values 70,75
        } else if (i >= 10 && i <= 12) {   // another cluster in B
            vecB[i] = 60 + (i - 10) * 10;  // values 60,70,80
        } else {
            vecB[i] = 0;                    // sparse zeros elsewhere
        }
    }

    // Compute an "alignment" metric:
    // sum of absolute differences between corresponding elements
    int alignScore = 0;

    // Manual loop unrolling – process 4 elements per iteration
    int i = 0;
    int limit = elemCount - (elemCount % 4);
    for (; i < limit; i += 4) {
        // First element
        int diff0 = vecA[i] - vecB[i];
        alignScore = alignScore + (diff0 < 0 ? -diff0 : diff0);

        // Second element
        int diff1 = vecA[i + 1] - vecB[i + 1];
        alignScore = alignScore + (diff1 < 0 ? -diff1 : diff1);

        // Third element
        int diff2 = vecA[i + 2] - vecB[i + 2];
        alignScore = alignScore + (diff2 < 0 ? -diff2 : diff2);

        // Fourth element
        int diff3 = vecA[i + 3] - vecB[i + 3];
        alignScore = alignScore + (diff3 < 0 ? -diff3 : diff3);
    }

    // Handle any remaining elements
    for (; i < elemCount; ++i) {
        int diff = vecA[i] - vecB[i];
        alignScore = alignScore + (diff < 0 ? -diff : diff);
    }

    // Print the generated arrays and the resulting alignment score
    std::cout << "Array A: ";
    for (int k = 0; k < elemCount; ++k) {
        std::cout << vecA[k] << (k + 1 == elemCount ? '\n' : ' ');
    }

    std::cout << "Array B: ";
    for (int k = 0; k < elemCount; ++k) {
        std::cout << vecB[k] << (k + 1 == elemCount ? '\n' : ' ');
    }

    std::cout << "Alignment Score (sum of |A[i]-B[i]|): " << alignScore << std::endl;

    // Clean up heap memory
    delete[] vecA;
    delete[] vecB;

    return 0;
}
