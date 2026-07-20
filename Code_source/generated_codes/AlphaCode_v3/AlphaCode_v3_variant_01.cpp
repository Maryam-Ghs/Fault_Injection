#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */
int main() {
    // --------------------------------------------------------------
    // Version #3 of AlphaCode – all logic is inside main()
    // --------------------------------------------------------------

    // 1. Initialise the random number generator
    std::srand(static_cast<int>(std::time(0)));

    // 2. Define the size of the test data (minimal boundary size)
    int arraySize = 1;                     // smallest nontrivial size

    // 3. Allocate the array on the heap
    int* dataPtr = new int[arraySize];

    // 4. Fill the array with random values in the range [0, 99]
    int fillIndex = 0;
    while (fillIndex < arraySize) {
        // generate a random number and store it
        int randomValue = std::rand() % 100;
        dataPtr[fillIndex] = randomValue;
        ++fillIndex;
    }

    // 5. Verbose step‑by‑step processing
    //    a) Compute the sum of the elements
    int totalSum = 0;
    int sumIdx = 0;
    while (sumIdx < arraySize) {
        // reordered arithmetic: add current element to the sum
        totalSum = totalSum + dataPtr[sumIdx];
        ++sumIdx;
    }

    //    b) Find the maximum value
    int maxVal = dataPtr[0];
    int maxIdx = 1;
    while (maxIdx < arraySize) {
        // reorder comparison to illustrate flexibility
        if (dataPtr[maxIdx] > maxVal) {
            maxVal = dataPtr[maxIdx];
        }
        ++maxIdx;
    }

    //    c) Find the minimum value
    int minVal = dataPtr[0];
    int minIdx = 1;
    while (minIdx < arraySize) {
        // reorder comparison similarly
        if (dataPtr[minIdx] < minVal) {
            minVal = dataPtr[minIdx];
        }
        ++minIdx;
    }

    //    d) Compute the average as a float (reordered arithmetic)
    float avgVal = 0.0f;
    // (totalSum / arraySize) is integer division; cast to float first
    avgVal = static_cast<float>(totalSum) / static_cast<float>(arraySize);

    // 6. Sort the array using a simple bubble sort (while‑loop version)
    int pass = 0;
    while (pass < arraySize - 1) {
        int inner = 0;
        while (inner < arraySize - pass - 1) {
            // reorder the swap condition to showcase flexibility
            if (dataPtr[inner] > dataPtr[inner + 1]) {
                // temporary holder for swapping
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
    // Re‑print the original random numbers (we have them now sorted,
    // so we regenerate a fresh copy for display purposes)
    // To stay within constraints, we reuse the same array before sorting:
    // (re‑fill it with the same random sequence by reseeding)
    std::srand(static_cast<int>(std::time(0)));
    fillIndex = 0;
    while (fillIndex < arraySize) {
        int randomValue = std::rand() % 100;
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
