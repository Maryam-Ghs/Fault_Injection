// LLM input variant 10: large-safe-stress
// AlphaCode – version #7
// Implements deterministic vector operations using heap allocation,
// loop‑heavy iteration, temporary variables and helper functions.

#include <iostream>
#include <cmath>

// ---------------------------------------------------------------------
// Helper: allocate a float array on the heap
float* allocate_buffer(int sz) {
    float* buf = new float[sz];
    return buf;
}

// Helper: release a heap‑allocated float array
void release_buffer(float* buf) {
    delete[] buf;
}

// Helper: fill an array with a deterministic pattern
void fill_pattern(float* buf, int sz, float scale) {
    int idx = 0;
    while (idx < sz) {
        // pattern: (index + 1) * scale
        buf[idx] = (float)(idx + 1) * scale;
        ++idx;
    }
}

// Helper: compute dot product of two arrays
float dot_product(const float* x, const float* y, int sz) {
    float acc = 0.0f;
    int i = 0;
    while (i < sz) {
        // split multiplication into a temporary variable
        float prod = x[i] * y[i];
        acc += prod;               // accumulate
        ++i;
    }
    return acc;
}

// Helper: compute sum of squares of an array
float sum_of_squares(const float* src, int sz) {
    float total = 0.0f;
    int j = sz - 1;                // iterate backwards (reordered)
    while (j >= 0) {
        float sq = src[j] * src[j];
        total += sq;
        --j;
    }
    return total;
}

// Helper: print a single floating‑point result
void print_result(const char* label, float value) {
    std::cout << label << ": " << value << '\n';
}

// ---------------------------------------------------------------------
// Main entry point – no external input, everything generated internally
int main() {
    // -------------------------------------------------------------
    // Configuration (deterministic test vectors)
    const int LENGTH = 2000000;          // size of the vectors (large but safe)
    const float SCALE_A = 1.5f;    // scaling factor for first vector
    const float SCALE_B = 2.0f;    // scaling factor for second vector

    // -------------------------------------------------------------
    // Allocate buffers on the heap
    float* vec_a = allocate_buffer(LENGTH);
    float* vec_b = allocate_buffer(LENGTH);

    // -------------------------------------------------------------
    // Fill buffers with deterministic data
    fill_pattern(vec_a, LENGTH, SCALE_A);
    fill_pattern(vec_b, LENGTH, SCALE_B);

    // -------------------------------------------------------------
    // Compute required metrics using helper functions
    float dp = dot_product(vec_a, vec_b, LENGTH);
    float sos = sum_of_squares(vec_a, LENGTH);
    float norm_a = sqrtf(sos);   // Euclidean norm (float version)

    // -------------------------------------------------------------
    // Output the results
    print_result("Dot product (A·B)", dp);
    print_result("Sum of squares of A", sos);
    print_result("Euclidean norm of A", norm_a);

    // -------------------------------------------------------------
    // Clean up heap memory
    release_buffer(vec_a);
    release_buffer(vec_b);

    return 0;
}
