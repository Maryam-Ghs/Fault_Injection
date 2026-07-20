/* LLM input variant 8: sparse-skewed */
// Version #10 – Adjacency Matrix Normalization (row‑wise)
// -----------------------------------------------------
// Generates a random adjacency matrix, computes row sums,
// and produces a row‑normalized matrix.  All logic lives
// inside main() and uses only int and float types.

#include <bits/stdc++.h>
using namespace std;

int main() {
    // ---------- configuration ----------
    int size = 10;                    // matrix dimension (expanded for sparsity)
    int maxWeight = 9;                // maximum random edge weight

    // ---------- generate deterministic sparse‑skewed adjacency matrix ----------
    srand(12345);                     // deterministic seed for reproducibility
    vector< vector<float> > adj(size, vector<float>(size));
    for (int r = 0; r < size; ++r) {
        // Probability of a non‑zero entry decreases with row index (skewed)
        int prob = max(5, (size - r) * 10); // between 5% and 100%
        for (int c = 0; c < size; ++c) {
            if (rand() % 100 < prob) {
                // random integer in [0, maxWeight], stored as float
                adj[r][c] = static_cast<float>(rand() % (maxWeight + 1));
            } else {
                adj[r][c] = 0.0f; // sparse zero
            }
        }
    }

    // ---------- display original matrix ----------
    printf("Original adjacency matrix (%dx%d):\n", size, size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            printf("%5.1f ", adj[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    // ---------- compute row sums (degree vector) ----------
    vector<float> rowSum(size, 0.0f);
    for (int i = 0; i < size; ++i) {
        // manual unrolling: process two elements per iteration
        int j = 0;
        for (; j + 1 < size; j += 2) {
            float a = adj[i][j];
            float b = adj[i][j + 1];
            // expanded multi‑step accumulation
            float temp = a + b;
            rowSum[i] = rowSum[i] + temp;
        }
        // handle possible leftover column
        if (j < size) {
            rowSum[i] = rowSum[i] + adj[i][j];
        }
    }

    // ---------- row‑normalize the matrix ----------
    vector< vector<float> > norm(size, vector<float>(size, 0.0f));
    for (int i = 0; i < size; ++i) {
        // avoid division by zero
        float invDenom = (rowSum[i] != 0.0f) ? (1.0f / rowSum[i]) : 0.0f;

        // manual unrolling again for the inner loop
        int k = 0;
        for (; k + 1 < size; k += 2) {
            // expanded steps: fetch, multiply, store
            float val0 = adj[i][k];
            float val1 = adj[i][k + 1];
            float norm0 = val0 * invDenom;
            float norm1 = val1 * invDenom;
            norm[i][k]     = norm0;
            norm[i][k + 1] = norm1;
        }
        if (k < size) {
            float val = adj[i][k];
            norm[i][k] = val * invDenom;
        }
    }

    // ---------- display normalized matrix ----------
    printf("Row‑normalized matrix:\n");
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            printf("%5.3f ", norm[i][j]);
        }
        printf("\n");
    }

    return 0;
}
