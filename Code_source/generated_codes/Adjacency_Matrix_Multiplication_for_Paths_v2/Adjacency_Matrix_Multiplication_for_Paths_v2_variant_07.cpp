#include <iostream>

/* LLM input variant 7: reverse-adversarial */

int main() {
    // matrix dimension (larger to increase density)
    int sz = 6;

    // allocate matrices on the heap
    int *mat1 = new int[sz * sz];
    int *mat2 = new int[sz * sz];
    int *prod = new int[sz * sz];

    // generate deterministic dense adjacency matrix in reverse order
    int idx = sz * sz - 1;
    while (idx >= 0) {
        mat1[idx] = 1; // fully connected graph
        --idx;
    }

    // copy mat1 into mat2 in reverse order (preserve original for multiplication)
    idx = sz * sz - 1;
    while (idx >= 0) {
        mat2[idx] = mat1[idx];
        --idx;
    }

    // initialise product matrix to zero
    idx = 0;
    while (idx < sz * sz) {
        prod[idx] = 0;
        ++idx;
    }

    // adjacency‑matrix multiplication (paths of length 2)
    int i = 0;
    while (i < sz) {
        int k = 0;
        while (k < sz) {
            int aik = mat2[i * sz + k];               // element from left matrix
            int j = 0;
            while (j < sz) {
                prod[i * sz + j] += aik * mat1[k * sz + j]; // fused multiply‑add
                ++j;
            }
            ++k;
        }
        ++i;
    }

    // output the resulting matrix
    i = 0;
    while (i < sz) {
        int j = 0;
        while (j < sz) {
            std::cout << prod[i * sz + j] << (j + 1 == sz ? '\n' : ' ');
            ++j;
        }
        ++i;
    }

    // clean up heap memory
    delete[] mat1;
    delete[] mat2;
    delete[] prod;

    return 0;
}
