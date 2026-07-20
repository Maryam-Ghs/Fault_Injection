// LLM input variant 2: small-diverse
// version #7 – Adjacency Matrix Multiplication for Paths
// -----------------------------------------------------
// Generates two deterministic adjacency matrices on the stack,
// multiplies them to count length‑2 paths, and prints the result.
// Only int is used; no const, unsigned, double, or long.

#include <iostream>
#include <cstdlib>
#include <ctime>

#define MAX_N 10   // maximum dimension, stack‑allocated

// Fill a square matrix with deterministic 0/1 values (directed graph)
void buildDeterministic(int sz, int mat[][MAX_N])
{
    // Example pattern: alternating rows of 0/1 with a shift
    for (int r = 0; r < sz; ++r)
        for (int c = 0; c < sz; ++c)
            mat[r][c] = ((r + c) % 3 == 0) ? 1 : 0;
}

// Multiply two adjacency matrices: res = left * right
// Loop order i‑k‑j gives good cache behaviour and fuses the inner work.
void multiply(int sz, int left[][MAX_N], int right[][MAX_N], int res[][MAX_N])
{
    // initialise result to zero
    for (int i = 0; i < sz; ++i)
        for (int j = 0; j < sz; ++j)
            res[i][j] = 0;

    // core multiplication
    for (int i = 0; i < sz; ++i)
        for (int k = 0; k < sz; ++k)
        {
            int aik = left[i][k];
            // fuse the inner product into a single statement
            for (int j = 0; j < sz; ++j)
                res[i][j] += aik * right[k][j];
        }
}

// Print a square matrix
void show(int sz, int mat[][MAX_N])
{
    for (int i = 0; i < sz; ++i)
    {
        for (int j = 0; j < sz; ++j)
            std::cout << mat[i][j] << ' ';
        std::cout << '\n';
    }
    std::cout << std::endl;
}

int main()
{
    // deterministic seed not needed; we use fixed data
    // Choose a size (between 3 and MAX_N)
    int dim = 5;                     // small diverse test
    int adjA[MAX_N][MAX_N];
    int adjB[MAX_N][MAX_N];
    int paths[MAX_N][MAX_N];

    // generate two deterministic adjacency matrices
    buildDeterministic(dim, adjA);
    buildDeterministic(dim, adjB);

    std::cout << "Adjacency matrix A:\n";
    show(dim, adjA);

    std::cout << "Adjacency matrix B:\n";
    show(dim, adjB);

    // compute A * B – number of length‑2 paths from A to B
    multiply(dim, adjA, adjB, paths);

    std::cout << "Product matrix (paths of length 2):\n";
    show(dim, paths);

    return 0;
}
