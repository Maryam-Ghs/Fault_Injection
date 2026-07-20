#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 2: small-diverse */

#define MAX_N 20          // maximum dimension (stack allocation)
#define ROWS   7           // actual size used for the demo

// ------------------------------------------------------------
// Fill the matrix with deterministic diverse values
// ------------------------------------------------------------
void generate_weights(int sz, float mat[][MAX_N])
{
    int r = 0;
    while (r < sz) {
        int c = 0;
        while (c < sz) {
            // Introduce zeros periodically and varied positive values
            if ((r + c) % 4 == 0)
                mat[r][c] = 0.0f;
            else
                mat[r][c] = static_cast<float>((r * c) % 9 + 1);
            ++c;
        }
        ++r;
    }
}

// ------------------------------------------------------------
// Compute row sums and store them in a temporary array
// ------------------------------------------------------------
void compute_row_sums(int sz, float mat[][MAX_N], float sums[])
{
    int i = 0;
    while (i < sz) {
        float acc = 0.0f;
        int j = 0;
        while (j < sz) {
            acc += mat[i][j];
            ++j;
        }
        sums[i] = acc;
        ++i;
    }
}

// ------------------------------------------------------------
// Normalize each row so that its elements add up to 1.0 (if possible)
// ------------------------------------------------------------
void normalize_rows(int sz, float mat[][MAX_N], const float sums[])
{
    int i = 0;
    while (i < sz) {
        // avoid division by zero
        if (sums[i] != 0.0f) {
            int j = 0;
            while (j < sz) {
                // mathematically equivalent re‑ordering:
                // (mat[i][j] / sums[i]) == mat[i][j] * (1.0f / sums[i])
                mat[i][j] = mat[i][j] / sums[i];
                ++j;
            }
        }
        ++i;
    }
}

// ------------------------------------------------------------
// Print the matrix to standard output
// ------------------------------------------------------------
void dump_matrix(int sz, float mat[][MAX_N])
{
    int i = 0;
    while (i < sz) {
        int j = 0;
        while (j < sz) {
            std::cout << mat[i][j];
            if (j < sz - 1) std::cout << ' ';
            ++j;
        }
        std::cout << '\n';
        ++i;
    }
}

// ------------------------------------------------------------
// Entry point – version #9
// ------------------------------------------------------------
int main()
{
    // ---- configuration ------------------------------------------------
    int dim = ROWS;                     // size of the square matrix
    float adjacency[MAX_N][MAX_N];      // stack‑allocated matrix
    float row_sum[MAX_N];               // temporary storage for row sums

    // ---- generate a deterministic adjacency matrix --------------------
    generate_weights(dim, adjacency);

    // ---- compute row sums -----------------------------------------------
    compute_row_sums(dim, adjacency, row_sum);

    // ---- normalize rows -------------------------------------------------
    normalize_rows(dim, adjacency, row_sum);

    // ---- output the normalized matrix -----------------------------------
    std::cout << "Adjacency matrix after normalization (version #9):\n";
    dump_matrix(dim, adjacency);

    return 0;
}
