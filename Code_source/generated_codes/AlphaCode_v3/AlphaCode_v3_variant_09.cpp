#include <iostream>
#include <cstdlib>

/* LLM input variant 9: medium-deterministic-random */

static unsigned int lcg_state = 0;
const unsigned int INITIAL_SEED = 123456789U;

unsigned int lcg_rand() {
    // Simple linear congruential generator (LCG)
    lcg_state = (1103515245U * lcg_state + 12345U) & 0x7fffffffU;
    return lcg_state;
}

int main() {
    // --------------------------------------------------------------
    // Version #3 of AlphaCode – all logic is inside main()
    // --------------------------------------------------------------

    // 1. Initialise deterministic pseudo‑random generator
    lcg_state = INITIAL_SEED;

    // 2. Define the size of the test data (medium random array)
    int arraySize = 20;                     // medium size

    // 3. Allocate the array on the heap
    int* dataPtr = new int[arraySize];

    // 4. Fill the array with deterministic pseudo‑random values in the range [0, 99]
    int fillIndex = 0;
    while (fillIndex < arraySize) {
        unsigned int randomValue = lcg_rand() % 100;
        dataPtr[fillIndex] = static_cast<int>(randomValue);
        ++fillIndex;
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
    // Re‑print the original deterministic sequence
    lcg_state = INITIAL_SEED;
    fillIndex = 0;
    while (fillIndex < arraySize) {
        unsigned int randomValue = lcg_rand() % 100;
        std::cout << randomValue << " ";
        ++fillIndex;
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

    return 0;
}
