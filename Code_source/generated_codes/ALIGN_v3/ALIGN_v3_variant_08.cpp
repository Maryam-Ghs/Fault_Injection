#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 8: sparse-skewed */

// ------------------------------------------------------------
// Helper: create a dynamic array filled with pseudo‑random ints
// ------------------------------------------------------------
int* buildArray(int length, int maxVal) {
    int* raw = new int[length];
    int i = 0;
    while (i < length) {
        raw[i] = std::rand() % maxVal;
        ++i;
    }
    return raw;
}

// ------------------------------------------------------------
// Helper: align a single integer to the next multiple of 'step'
// ------------------------------------------------------------
int alignOne(int value, int step) {
    // expanded multi‑step computation
    int remainder = value % step;
    int need = step - remainder;
    // if already aligned, need becomes step; fix it
    if (remainder == 0) need = 0;
    int aligned = value + need;
    return aligned;
}

// ------------------------------------------------------------
// Helper: produce a new array where each element is aligned
// ------------------------------------------------------------
int* alignArray(const int* source, int length, int step) {
    int* aligned = new int[length];
    int idx = length - 1;                     // reverse traversal
    while (idx >= 0) {
        // reorder: compute aligned value after fetching source
        int original = source[idx];
        int alignedVal = alignOne(original, step);
        aligned[idx] = alignedVal;
        --idx;
    }
    return aligned;
}

// ------------------------------------------------------------
// Helper: display the content of an array
// ------------------------------------------------------------
void showArray(const int* arr, int length, const char* label) {
    std::cout << label << " (size " << length << "):\n";
    int pos = 0;
    while (pos < length) {
        std::cout << arr[pos];
        if (pos != length - 1) std::cout << ", ";
        ++pos;
        // break line every 20 numbers for readability
        if (pos % 20 == 0) std::cout << "\n";
    }
    std::cout << "\n\n";
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main() {
    std::srand(static_cast<unsigned>(std::time(0)));

    // sparse-skewed input parameters
    int sizeN = 1024;          // larger size to allow sparsity
    int maxRand = 1000;        // range for random generation (will be overwritten)
    int alignStep = 8;         // alignment boundary

    // generate raw data
    int* rawData = buildArray(sizeN, maxRand);

    // impose sparsity: mostly zeros with a few clustered non‑zero values
    for (int i = 0; i < sizeN; ++i) {
        rawData[i] = 0;
    }
    // cluster of non‑zero values near the beginning
    rawData[2]   = 987;
    rawData[5]   = 123;
    rawData[7]   = 456;
    rawData[9]   = 789;
    // isolated non‑zero values far apart
    rawData[200] = 654;
    rawData[500] = 321;
    rawData[800] = 111;
    rawData[950] = 999;

    showArray(rawData, sizeN, "Original data");

    // align the data
    int* alignedData = alignArray(rawData, sizeN, alignStep);
    showArray(alignedData, sizeN, "Aligned data");

    // clean up
    delete[] rawData;
    delete[] alignedData;

    return 0;
}
