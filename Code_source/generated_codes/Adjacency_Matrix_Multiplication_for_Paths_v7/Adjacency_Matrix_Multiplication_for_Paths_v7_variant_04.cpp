/* LLM input variant 4: signed-extremes */
// version #7 – Adjacency Matrix Multiplication for Paths
// -----------------------------------------------------
// Generates two deterministic adjacency matrices on the stack,
// mixes negative, zero, and positive values, multiplies them to count
// length‑2 paths, and prints the result.
// Only int is used; no const, unsigned, double, or long.

#include <iostream>
#include <cstdlib>
#include <ctime>

#define MAX_N 10   // maximum dimension, stack‑allocated

// Fill a square matrix with deterministic values -1, 0, or 1
void buildRandom(int sz, int mat[][MAX_N])
{
    for (int r = 0; r < sz; ++r)
        for (int c = 0; c < sz; ++c)
            mat[r][c] = (std::rand() % 3) - 1; // yields -1, 0, or 1
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
    // Fixed seed for deterministic behavior
    std::srand(123456789);

    // Choose a size (between 3 and MAX_N)
    int dim = 5;                     // changed to a smaller size for clearer output
    int adjA[MAX_N][MAX_N];
    int adjB[MAX_N][MAX_N];
    int paths[MAX_N][MAX_N];

    // generate two independent adjacency matrices with mixed signs
    buildRandom(dim, adjA);
    buildRandom(dim, adjB);

    std::cout << "Adjacency matrix A:\n";
    show(dim, adjA);

    std::cout << "Adjacency matrix B:\n";
    show(dim, adjB);

    // compute A * B – number of length‑2 paths from A to B (signed counts)
    multiply(dim, adjA, adjB, paths);

    std::cout << "Product matrix (paths of length 2):\n";
    show(dim, paths);

    return 0;
}
