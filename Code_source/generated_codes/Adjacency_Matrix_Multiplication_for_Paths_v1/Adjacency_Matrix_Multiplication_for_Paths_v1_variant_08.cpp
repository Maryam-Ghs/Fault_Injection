#include <iostream>
using namespace std;

/* LLM input variant 8: sparse-skewed */

/* version #1 - adjacency matrix multiplication (paths) */

int N = 6;                                 // size of the square matrix

/* fill a small predefined sparse and skewed adjacency matrix */
void buildAdjacency(int adj[7][7])
{
    // clear matrix
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            adj[r][c] = 0;

    // sparse, imbalanced edges
    adj[0][1] = 1;
    adj[0][2] = 1;
    adj[0][3] = 1;   // node 0 has several outgoing edges
    adj[1][5] = 1;   // node 1 points to a high-index node
    adj[4][0] = 1;   // isolated node 4 points back to 0
    adj[5][5] = 1;   // self-loop on node 5
}

/* multiply two matrices: res = left * right   (order of loops: i,k,j) */
void multiply(int left[7][7], int right[7][7], int res[7][7])
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
void showMatrix(const char* title, int mat[7][7])
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
    int adj[7][7];
    int square[7][7];
    int pathsLen2[7][7];

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
