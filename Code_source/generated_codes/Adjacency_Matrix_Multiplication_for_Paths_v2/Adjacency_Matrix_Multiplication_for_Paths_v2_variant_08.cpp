/* LLM input variant 8: sparse-skewed */
#include <iostream>

int main() {
    // matrix dimension (larger size to emphasize sparsity)
    int sz = 10;

    // allocate matrices on the heap
    int *mat1 = new int[sz * sz];
    int *mat2 = new int[sz * sz];
    int *prod = new int[sz * sz];

    // generate deterministic sparse adjacency matrix (clustered non‑zeros in top‑left corner)
    int idx = 0;
    while (idx < sz * sz) {
        int row = idx / sz;
        int col = idx % sz;
        // only a small 2x2 block has ones; rest are zeros
        mat1[idx] = (row < 2 && col < 2) ? 1 : 0;
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
