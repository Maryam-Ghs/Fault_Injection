#include <iostream>

/* LLM input variant 8: sparse-skewed */

int main()
{
    // -------------------------------------------------
    // 1. Define dimensions and allocate matrices on heap
    // -------------------------------------------------
    int sz = 7;                                   // size of the square adjacency matrix
    float* rawMat = new float[sz * sz];           // original adjacency matrix (flattened)
    float* normMat = new float[sz * sz];          // normalized matrix (flattened)

    // -------------------------------------------------
    // 2. Fill the original matrix with a deterministic sparse‑skewed pattern
    // -------------------------------------------------
    // Initialize all entries to 0
    for (int i = 0; i < sz * sz; ++i)
        rawMat[i] = 0.0f;

    // Row 0: dense (all ones)
    for (int c = 0; c < sz; ++c)
        rawMat[0 * sz + c] = 1.0f;

    // Row 1: a single one near the start
    rawMat[1 * sz + 0] = 1.0f;

    // Row 2: a single one near the end
    rawMat[2 * sz + 6] = 1.0f;

    // Row 4: a couple of scattered ones
    rawMat[4 * sz + 2] = 1.0f;
    rawMat[4 * sz + 5] = 1.0f;

    // Rows 3,5,6 remain all zeros (empty rows)

    // -------------------------------------------------
    // 3. Print the generated adjacency matrix
    // -------------------------------------------------
    std::cout << "Original adjacency matrix (" << sz << "x" << sz << "):\n";
    for (int r = 0; r < sz; ++r)
    {
        for (int c = 0; c < sz; ++c)
        {
            std::cout << rawMat[r * sz + c] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "\n";

    // -------------------------------------------------
    // 4. Row‑wise normalization:
    //    each entry = entry / (sum of its row)
    // -------------------------------------------------
    int row = 0;
    while (row < sz)
    {
        // ---- 4.1 Compute the sum of the current row ----
        float rowSum = 0.0f;
        int col = 0;
        while (col < sz)
        {
            rowSum = rowSum + rawMat[row * sz + col];
            ++col;
        }

        // ---- 4.2 Guard against division by zero (empty row) ----
        if (rowSum == 0.0f)
        {
            // If the row is all zeros, keep the normalized row as zeros
            int k = 0;
            while (k < sz)
            {
                normMat[row * sz + k] = 0.0f;
                ++k;
            }
        }
        else
        {
            // ---- 4.3 Fill the normalized row ----
            int j = 0;
            while (j < sz)
            {
                float inv = 1.0f / rowSum;
                normMat[row * sz + j] = rawMat[row * sz + j] * inv;
                ++j;
            }
        }
        ++row;
    }

    // -------------------------------------------------
    // 5. Print the normalized matrix
    // -------------------------------------------------
    std::cout << "Row‑wise normalized matrix:\n";
    for (int i = 0; i < sz; ++i)
    {
        for (int j = 0; j < sz; ++j)
        {
            // Show numbers with three decimal places
            std::cout << static_cast<float>(static_cast<int>(normMat[i * sz + j] * 1000 + 0.5f)) / 1000 << ' ';
        }
        std::cout << '\n';
    }

    // -------------------------------------------------
    // 6. Clean up heap memory
    // -------------------------------------------------
    delete[] rawMat;
    delete[] normMat;

    return 0;
}
