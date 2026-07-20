#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */

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
    // deterministic seed for minimal test
    std::srand(0);

    // minimal input parameters
    int sizeN = 1;          // smallest non‑zero size
    int maxRand = 2;       // values in [0,1]
    int alignStep = 1;     // boundary alignment

    // generate raw data
    int* rawData = buildArray(sizeN, maxRand);
    showArray(rawData, sizeN, "Original data");

    // align the data
    int* alignedData = alignArray(rawData, sizeN, alignStep);
    showArray(alignedData, sizeN, "Aligned data");

    // clean up
    delete[] rawData;
    delete[] alignedData;

    return 0;
}
