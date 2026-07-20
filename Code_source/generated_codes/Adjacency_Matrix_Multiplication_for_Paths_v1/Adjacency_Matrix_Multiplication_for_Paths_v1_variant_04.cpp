#include <iostream>
using namespace std;

/* LLM input variant 4: signed-extremes */

/* version #1 - adjacency matrix multiplication (paths) */

int N = 4;                                 // size of the square matrix

/* fill a small predefined adjacency matrix with mixed signed values */
void buildAdjacency(int adj[5][5])
{
    /* example graph with negative, zero, and positive edge weights:
       0 -> 1 ( -1 ), 0 -> 2 ( 2 )
       1 -> 0 ( 0 ), 1 -> 2 ( -3 )
       2 -> 3 ( 4 )
       3 -> 0 ( -2 ), 3 -> 1 ( 0 )
    */
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            adj[r][c] = 0;                 // clear

    adj[0][1] = -1;
    adj[0][2] = 2;
    adj[1][0] = 0;
    adj[1][2] = -3;
    adj[2][3] = 4;
    adj[3][0] = -2;
    adj[3][1] = 0;
}

/* multiply two matrices: res = left * right   (order of loops: i,k,j) */
void multiply(int left[5][5], int right[5][5], int res[5][5])
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
void showMatrix(const char* title, int mat[5][5])
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
    int adj[5][5];
    int square[5][5];
    int pathsLen2[5][5];

    buildAdjacency(adj);
    showMatrix("Adjacency matrix (A):", adj);

    // A * A  -> number of paths of length 2 (with signed weights)
    multiply(adj, adj, pathsLen2);
    showMatrix("A squared (paths of length 2):", pathsLen2);

    // optional: demonstrate A * A * A (paths of length 3)
    multiply(pathsLen2, adj, square);
    showMatrix("A^3 (paths of length 3):", square);

    return 0;
}
