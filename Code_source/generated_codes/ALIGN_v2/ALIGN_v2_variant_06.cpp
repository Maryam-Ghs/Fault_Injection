#include <iostream>
/* LLM input variant 6: ordered-structured */

int main() {
    // Size of the arrays (medium)
    int elemCount = 23;                     // a medium‑sized structured array
    int *vecA = new int[elemCount];
    int *vecB = new int[elemCount];

    // Fill arrays with deterministic, highly structured data
    // vecA: ascending sequence 0,1,2,...
    // vecB: descending sequence elemCount-1, elemCount-2, ...
    for (int i = 0; i < elemCount; ++i) {
        vecA[i] = i;
        vecB[i] = elemCount - 1 - i;
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
