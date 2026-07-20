#include <iostream>
using namespace std;

/* LLM input variant 1: minimal-boundary */
/* version #1 - adjacency matrix multiplication (paths) */

int N = 1;                                 // size of the square matrix (minimal non‑trivial)

void buildAdjacency(int adj[2][2])
{
    // clear matrix
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            adj[r][c] = 0;

    // minimal graph: single node with a self‑loop
    adj[0][0] = 1;
}

/* multiply two matrices: res = left * right   (order of loops: i,k,j) */
void multiply(int left[2][2], int right[2][2], int res[2][2])
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
void showMatrix(const char* title, int mat[2][2])
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
    int adj[2][2];
    int square[2][2];
    int pathsLen2[2][2];

    buildAdjacency(adj);
    showMatrix("Adjacency matrix (A):", adj);

    // A * A  -> number of paths of length 2
    multiply(adj, adj, pathsLen2);
    showMatrix("A squared (paths of length 2):", pathsLen2);

    // demonstrate A * A * A (paths of length 3)
    multiply(pathsLen2, adj, square);
    showMatrix("A^3 (paths of length 3):", square);

    return 0;
}
