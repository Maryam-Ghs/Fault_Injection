#include <iostream>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // matrix dimension (medium size)
    const int sz = 10;

    // allocate matrices on the heap
    int *mat1 = new int[sz * sz];
    int *mat2 = new int[sz * sz];
    int *prod = new int[sz * sz];

    // deterministic pseudo‑random generator (linear congruential)
    const unsigned int a = 1664525u;
    const unsigned int c = 1013904223u;
    const unsigned int m = 0xFFFFFFFFu;
    unsigned int seed = 123456789u;

    // generate deterministic adjacency matrix with values 0‑4
    for (int idx = 0; idx < sz * sz; ++idx) {
        seed = (a * seed + c) & m;
        mat1[idx] = static_cast<int>((seed >> 16) % 5); // values 0..4
    }

    // copy mat1 into mat2 (preserve original for multiplication)
    for (int idx = 0; idx < sz * sz; ++idx) {
        mat2[idx] = mat1[idx];
    }

    // initialise product matrix to zero
    for (int idx = 0; idx < sz * sz; ++idx) {
        prod[idx] = 0;
    }

    // adjacency‑matrix multiplication (paths of length 2)
    for (int i = 0; i < sz; ++i) {
        for (int k = 0; k < sz; ++k) {
            int aik = mat2[i * sz + k];               // element from left matrix
            for (int j = 0; j < sz; ++j) {
                prod[i * sz + j] += aik * mat1[k * sz + j]; // fused multiply‑add
            }
        }
    }

    // output the resulting matrix
    for (int i = 0; i < sz; ++i) {
        for (int j = 0; j < sz; ++j) {
            std::cout << prod[i * sz + j] << (j + 1 == sz ? '\n' : ' ');
        }
    }

    // clean up heap memory
    delete[] mat1;
    delete[] mat2;
    delete[] prod;

    return 0;
}
