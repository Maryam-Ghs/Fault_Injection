/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <cstdlib>
#include <ctime>

#define MAX_N 120          // maximum dimension (stack allocation)
#define ROWS   50          // actual size used for the demo

// ------------------------------------------------------------
// Fill the matrix with a sparse, clustered pattern of integer weights (converted to float)
// ------------------------------------------------------------
void generate_weights(int sz, float mat[][MAX_N])
{
    // Deterministic sparse pattern:
    // Most entries are zero. Non‑zero weights appear in clustered blocks:
    // - Every 10th row and column gets a small dense block of size 3x3 with values 1‑9.
    // - Additionally, a single heavy cluster is placed in the top‑left corner.
    for (int r = 0; r < sz; ++r) {
        for (int c = 0; c < sz; ++c) {
            mat[r][c] = 0.0f; // default zero
        }
    }

    // Heavy cluster in top‑left 5x5 area
    for (int r = 0; r < 5 && r < sz; ++r) {
        for (int c = 0; c < 5 && c < sz; ++c) {
            mat[r][c] = static_cast<float>((r + c) % 9 + 1); // values 1‑9
        }
    }

    // Small 3x3 clusters every 10 rows/cols
    for (int base = 0; base < sz; base += 10) {
        for (int dr = 0; dr < 3 && base + dr < sz; ++dr) {
            for (int dc = 0; dc < 3 && base + dc < sz; ++dc) {
                // Skip the area already filled by the heavy cluster to avoid overlap
                if (base < 5 && dr < 5 && base + dr < 5 && dc < 5) continue;
                mat[base + dr][base + dc] = static_cast<float>((dr * dc) % 9 + 1);
            }
        }
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

    // ---- generate a sparse, clustered adjacency matrix ----------------------------
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
