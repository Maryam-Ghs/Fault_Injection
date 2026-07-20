#include <iostream>
using namespace std;

/* LLM input variant 9: medium-deterministic-random */

/* version #1 - adjacency matrix multiplication (paths) */

int N = 7;                                 // size of the square matrix

/* fill a deterministic pseudo‑random adjacency matrix */
void buildAdjacency(int adj[8][8])
{
    // clear matrix
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            adj[r][c] = 0;

    // deterministic pattern: edge exists if (i*i + j*j + i*j) mod 4 == 0,
    // but avoid self‑loops.
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            if (i != j && ((i*i + j*j + i*j) % 4 == 0))
                adj[i][j] = 1;
}

/* multiply two matrices: res = left * right   (order of loops: i,k,j) */
void multiply(int left[8][8], int right[8][8], int res[8][8])
{
    // initialise result matrix to zero
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            res[i][j] = 0;

    // core multiplication with reordered arithmetic
    for (int i = 0; i < N; ++i)
    {
        int k = 0;
        while (k < N)
        {
            int temp = left[i][k];
            int j = 0;
            while (j < N)
            {
                int prod = temp * right[k][j];   // product first
                res[i][j] = res[i][j] + prod;    // then addition
                ++j;
            }
            ++k;
        }
    }
}

/* print a matrix to stdout */
void showMatrix(const char* title, int mat[8][8])
{
    cout << title << endl;
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            cout << mat[i][j] << ' ';
        }
        cout << endl;
    }
    cout << endl;
}

/* driver */
int main()
{
    int adj[8][8];
    int square[8][8];
    int pathsLen2[8][8];

    buildAdjacency(adj);
    showMatrix("Adjacency matrix (A):", adj);

    // A * A  -> number of paths of length 2
    multiply(adj, adj, pathsLen2);
    showMatrix("A squared (paths of length 2):", pathsLen2);

    // optional: demonstrate A * A * A (paths of length 3)
    multiply(pathsLen2, adj, square);
    showMatrix("A^3 (paths of length 3):", square);

    return 0;
}
