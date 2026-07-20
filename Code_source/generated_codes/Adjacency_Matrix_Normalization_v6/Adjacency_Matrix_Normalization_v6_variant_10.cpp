/* LLM input variant 10: large-safe-stress */
#include <cstdio>

enum { N = 100 };                     // matrix dimension (larger safe size)

void computeRowSums(int sz, float src[][N], float sums[])
{
    int idxRow = 0;
    while (idxRow < sz)               // outer while loop
    {
        int idxCol = 0;
        float acc = 0.0f;
        while (idxCol < sz)           // inner while loop
        {
            // reordered addition (value first, then accumulator)
            acc = src[idxRow][idxCol] + acc;
            ++idxCol;
        }
        sums[idxRow] = acc;            // store the row sum
        ++idxRow;
    }
}

void normalizeRows(int sz, float src[][N], float sums[], float dst[][N])
{
    int idxRow = 0;
    while (idxRow < sz)
    {
        // fetch divisor once per row – reordered before inner loop
        float divisor = sums[idxRow];
        int idxCol = 0;
        while (idxCol < sz)
        {
            float val = src[idxRow][idxCol];
            // division only if divisor is non‑zero (avoid /0)
            if (divisor != 0.0f)
                dst[idxRow][idxCol] = val / divisor;   // normalized entry
            else
                dst[idxRow][idxCol] = 0.0f;
            ++idxCol;
        }
        ++idxRow;
    }
}

void printMatrix(int sz, float mat[][N])
{
    int i = 0;
    while (i < sz)
    {
        int j = 0;
        while (j < sz)
        {
            // print with 2 decimal places
            printf("%6.2f", mat[i][j]);
            ++j;
        }
        printf("\n");
        ++i;
    }
    printf("\n");
}

int main()
{
    // deterministic large adjacency matrix (example graph)
    float srcMat[N][N];
    int i = 0;
    while (i < N)
    {
        int j = 0;
        while (j < N)
        {
            if (i == j)
                srcMat[i][j] = 0.0f;
            else
                srcMat[i][j] = static_cast<float>(((i + j) % 10) + 1);
            ++j;
        }
        ++i;
    }

    float sumVec[N];          // row sums
    float normMat[N][N];      // normalized matrix

    // Step 1: compute row sums
    computeRowSums(N, srcMat, sumVec);

    // Step 2: normalize each row by its sum
    normalizeRows(N, srcMat, sumVec, normMat);

    // Output
    printf("Original adjacency matrix:\n");
    printMatrix(N, srcMat);

    printf("Row‑normalized adjacency matrix:\n");
    printMatrix(N, normMat);

    return 0;
}
