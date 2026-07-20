#include <iostream>

#define N 6

/* LLM input variant 3: zeros-and-ones */

// -----------------------------------------------------------------
// Fill a square matrix with a deterministic adjacency pattern.
// -----------------------------------------------------------------
void fillAdjacency(int mat[N][N], int sz)
{
    int r = 0, c = 0;
    while (r < sz)
    {
        c = 0;
        while (c < sz)
        {
            // Emphasize identity: 1 on the diagonal, 0 elsewhere
            mat[r][c] = (r == c) ? 1 : 0;
            ++c;
        }
        ++r;
    }
}

// -----------------------------------------------------------------
// Multiply two square matrices (A * B) -> C.
// -----------------------------------------------------------------
void multiply(const int A[N][N], const int B[N][N], int C[N][N], int sz)
{
    int i = 0, j = 0, k = 0;
    while (i < sz)
    {
        j = 0;
        while (j < sz)
        {
            int acc = 0;                     // accumulator for the dot product
            k = 0;
            while (k < sz)
            {
                int prod = A[i][k] * B[k][j];
                acc = acc + prod;            // expanded addition
                ++k;
            }
            C[i][j] = acc;                   // store after full accumulation
            ++j;
        }
        ++i;
    }
}

// -----------------------------------------------------------------
// Print a square matrix to std::cout.
// -----------------------------------------------------------------
void showMatrix(const int M[N][N], int sz)
{
    int r = 0, c = 0;
    while (r < sz)
    {
        c = 0;
        while (c < sz)
        {
            std::cout << M[r][c] << ' ';
            ++c;
        }
        std::cout << '\n';
        ++r;
    }
}

// -----------------------------------------------------------------
int main()
{
    int size = N;                     // matrix dimension
    int adj[N][N];                    // adjacency matrix (paths of length 1)
    int path2[N][N];                  // result matrix (paths of length 2)

    fillAdjacency(adj, size);         // generate deterministic test data
    std::cout << "Adjacency matrix (length‑1 paths):\n";
    showMatrix(adj, size);

    multiply(adj, adj, path2, size); // compute A^2

    std::cout << "\nMatrix of paths of length 2 (A^2):\n";
    showMatrix(path2, size);

    return 0;
}
