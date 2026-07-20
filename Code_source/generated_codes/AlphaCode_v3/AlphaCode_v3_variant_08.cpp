#include <iostream>

/* LLM input variant 8: sparse-skewed */

int main() {
    // --------------------------------------------------------------
    // Version #3 of AlphaCode – all logic is inside main()
    // --------------------------------------------------------------

    // 1. Define the size of the test data (medium array with sparse, skewed values)
    int arraySize = 20;                     // medium size

    // 2. Allocate the array on the heap
    int* dataPtr = new int[arraySize];

    // 3. Fill the array with a deterministic sparse‑skewed pattern:
    //    every 5th element gets a distinct large value, others are zero.
    int fillIndex = 0;
    while (fillIndex < arraySize) {
        if (fillIndex % 5 == 0) {
            dataPtr[fillIndex] = 100 + fillIndex; // clustered high values
        } else {
            dataPtr[fillIndex] = 0;                // majority zeros (sparse)
        }
        ++fillIndex;
    }

    // 4. Preserve a copy of the original (unsorted) data for later display
    int* originalCopy = new int[arraySize];
    int copyIdx = 0;
    while (copyIdx < arraySize) {
        originalCopy[copyIdx] = dataPtr[copyIdx];
        ++copyIdx;
    }

    // 5. Verbose step‑by‑step processing
    //    a) Compute the sum of the elements
    int totalSum = 0;
    int sumIdx = 0;
    while (sumIdx < arraySize) {
        totalSum = totalSum + dataPtr[sumIdx];
        ++sumIdx;
    }

    //    b) Find the maximum value
    int maxVal = dataPtr[0];
    int maxIdx = 1;
    while (maxIdx < arraySize) {
        if (dataPtr[maxIdx] > maxVal) {
            maxVal = dataPtr[maxIdx];
        }
        ++maxIdx;
    }

    //    c) Find the minimum value
    int minVal = dataPtr[0];
    int minIdx = 1;
    while (minIdx < arraySize) {
        if (dataPtr[minIdx] < minVal) {
            minVal = dataPtr[minIdx];
        }
        ++minIdx;
    }

    //    d) Compute the average as a float (reordered arithmetic)
    float avgVal = 0.0f;
    avgVal = static_cast<float>(totalSum) / static_cast<float>(arraySize);

    // 6. Sort the array using a simple bubble sort (while‑loop version)
    int pass = 0;
    while (pass < arraySize - 1) {
        int inner = 0;
        while (inner < arraySize - pass - 1) {
            if (dataPtr[inner] > dataPtr[inner + 1]) {
                int tmp = dataPtr[inner];
                dataPtr[inner] = dataPtr[inner + 1];
                dataPtr[inner + 1] = tmp;
            }
            ++inner;
        }
        ++pass;
    }

    // 7. Output the results
    std::cout << "AlphaCode Version #3 Results\n";
    std::cout << "-----------------------------\n";

    std::cout << "Original (unsorted) data: ";
    int outOrig = 0;
    while (outOrig < arraySize) {
        std::cout << originalCopy[outOrig] << " ";
        ++outOrig;
    }
    std::cout << "\n";

    std::cout << "Sorted data: ";
    int outIdx = 0;
    while (outIdx < arraySize) {
        std::cout << dataPtr[outIdx] << " ";
        ++outIdx;
    }
    std::cout << "\n";

    std::cout << "Sum: " << totalSum << "\n";
    std::cout << "Average: " << avgVal << "\n";
    std::cout << "Maximum: " << maxVal << "\n";
    std::cout << "Minimum: " << minVal << "\n";

    // 8. Clean up heap memory
    delete[] dataPtr;
    delete[] originalCopy;

    return 0;
}
