#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

int main() {
    // --------------------------------------------------------------
    // Version #3 of AlphaCode – all logic is inside main()
    // --------------------------------------------------------------

    // 1. Initialise the random number generator with a fixed seed for determinism
    std::srand(0);

    // 2. Define the size of the test data (large safe size)
    int arraySize = 10000;                     // large safe size

    // 3. Allocate the array on the heap
    int* dataPtr = new int[arraySize];

    // 4. Fill the array with deterministic values in the range [0, 99]
    for (int i = 0; i < arraySize; ++i) {
        dataPtr[i] = (i * 37) % 100;   // deterministic pseudo‑random pattern
    }

    // Keep a copy of the original unsorted data for later display
    int* originalCopy = new int[arraySize];
    for (int i = 0; i < arraySize; ++i) {
        originalCopy[i] = dataPtr[i];
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
    float avgVal = static_cast<float>(totalSum) / static_cast<float>(arraySize);

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
    for (int i = 0; i < arraySize; ++i) {
        std::cout << originalCopy[i] << " ";
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
