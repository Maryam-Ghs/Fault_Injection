#include <iostream>

/* LLM input variant 4: signed-extremes */

int main() {
    // matrix dimension
    int sz = 4;

    // allocate matrices on the heap
    int *mat1 = new int[sz * sz];
    int *mat2 = new int[sz * sz];
    int *prod = new int[sz * sz];

    // generate deterministic adjacency matrix with mixed signed values
    int idx = 0;
    while (idx < sz * sz) {
        int row = idx / sz;
        int col = idx % sz;
        // values cycle through -1, 0, 1
        mat1[idx] = ((row - col + 3) % 3) - 1;
        ++idx;
    }

    // copy mat1 into mat2 (preserve original for multiplication)
    idx = 0;
    while (idx < sz * sz) {
        mat2[idx] = mat1[idx];
        ++idx;
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
