/* LLM input variant 3: zeros-and-ones */
// Version #10 – Adjacency Matrix Normalization (row‑wise)
// -----------------------------------------------------
// Generates a deterministic adjacency matrix emphasizing zeros, ones, and identity values,
// computes row sums, and produces a row‑normalized matrix.  All logic lives
// inside main() and uses only int and float types.

#include <bits/stdc++.h>
using namespace std;

int main() {
    // ---------- configuration ----------
    int size = 5;                     // matrix dimension
    int maxWeight = 1;                // maximum edge weight (0 or 1)

    // ---------- generate deterministic adjacency matrix ----------
    srand(12345);                     // deterministic seed for reproducibility
    vector< vector<float> > adj(size, vector<float>(size));
    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            // identity‑like pattern: 1 on diagonal, 0 elsewhere
            adj[r][c] = (r == c) ? 1.0f : 0.0f;
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
