#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

int main() {
    // Deterministic seed for reproducibility
    std::srand(12345);

    // Size of the arrays (large but safe)
    const int elemCount = 10000; // large‑sized deterministic array
    int *vecA = new int[elemCount];
    int *vecB = new int[elemCount];

    // Fill both arrays with deterministic integers in range [0,99]
    for (int i = 0; i < elemCount; ++i) {
        vecA[i] = i % 100;                     // pattern 0..99 repeated
        vecB[i] = (i * 2) % 100;               // pattern 0,2,4,... repeated
    }

    // Compute an "alignment" metric:
    // sum of absolute differences between corresponding elements
    int alignScore = 0;

    // Manual loop unrolling – process 4 elements per iteration
    int i = 0;
    int limit = elemCount - (elemCount % 4);
    for (; i < limit; i += 4) {
        int diff0 = vecA[i] - vecB[i];
        alignScore += (diff0 < 0 ? -diff0 : diff0);

        int diff1 = vecA[i + 1] - vecB[i + 1];
        alignScore += (diff1 < 0 ? -diff1 : diff1);

        int diff2 = vecA[i + 2] - vecB[i + 2];
        alignScore += (diff2 < 0 ? -diff2 : diff2);

        int diff3 = vecA[i + 3] - vecB[i + 3];
        alignScore += (diff3 < 0 ? -diff3 : diff3);
    }

    // Handle any remaining elements
    for (; i < elemCount; ++i) {
        int diff = vecA[i] - vecB[i];
        alignScore += (diff < 0 ? -diff : diff);
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
