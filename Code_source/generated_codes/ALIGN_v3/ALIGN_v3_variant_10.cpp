#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// Helper: create a deterministic array filled with pseudo‑random ints
// ------------------------------------------------------------
int* buildArray(int length, int maxVal) {
    int* raw = new int[length];
    int i = 0;
    while (i < length) {
        // deterministic pattern instead of std::rand()
        raw[i] = (static_cast<long long>(i) * 1234567LL) % maxVal;
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
    // deterministic seed (not used in buildArray but kept for completeness)
    std::srand(12345);

    // large stress array parameters (still safe for typical environments)
    int sizeN = 200000;        // large size
    int maxRand = 1000000;     // values will be in [0, 999999]
    int alignStep = 64;        // larger alignment boundary

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
