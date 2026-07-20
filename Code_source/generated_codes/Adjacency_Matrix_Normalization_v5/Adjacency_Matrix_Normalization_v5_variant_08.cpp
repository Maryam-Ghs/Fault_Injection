/* LLM input variant 8: sparse-skewed */
// Version 5 – Adjacency Matrix Normalization (class‑based, heap allocation, verbose)

#include <iostream>
#include <cstdlib>
#include <ctime>

class MatrixNormalizer {
public:
    // Constructor: allocate matrices on the heap
    MatrixNormalizer(int sz) : matrixSize(sz) {
        // Allocate raw adjacency matrix (integer weights)
        rawAdj = new int*[matrixSize];
        for (int i = 0; i < matrixSize; ++i) {
            rawAdj[i] = new int[matrixSize];
        }

        // Allocate normalized adjacency matrix (float values)
        normAdj = new float*[matrixSize];
        for (int i = 0; i < matrixSize; ++i) {
            normAdj[i] = new float[matrixSize];
        }
    }

    // Destructor: free all heap memory
    ~MatrixNormalizer() {
        for (int i = 0; i < matrixSize; ++i) {
            delete[] rawAdj[i];
            delete[] normAdj[i];
        }
        delete[] rawAdj;
        delete[] normAdj;
    }

    // Populate raw adjacency matrix with a deterministic sparse‑skewed pattern
    void generateRandomData() {
        // Create a highly sparse matrix: only diagonal and next‑column entries are non‑zero.
        // Values are skewed: early rows have larger weights, later rows smaller.
        for (int i = 0; i < matrixSize; ++i) {
            for (int j = 0; j < matrixSize; ++j) {
                rawAdj[i][j] = 0; // default to zero (sparsity)
            }
        }

        for (int i = 0; i < matrixSize; ++i) {
            // Diagonal entry gets a decreasing weight to create skew
            rawAdj[i][i] = matrixSize - i; // e.g., 20,19,...,1

            // Also set the entry just after the diagonal (wrap around) to a small constant
            int nextCol = (i + 1) % matrixSize;
            rawAdj[i][nextCol] = 1;
        }

        // Ensure each row sum is non‑zero (already guaranteed by diagonal)
    }

    // Perform row‑wise normalization
    void normalizeRows() {
        int rowIdx = 0;
        while (rowIdx < matrixSize) {
            // ---- Step 1: compute the sum of the current row ----
            int rowAccumulator = 0;
            int colIdx = 0;
            while (colIdx < matrixSize) {
                rowAccumulator += rawAdj[rowIdx][colIdx];
                ++colIdx;
            }

            // ---- Step 2: convert the integer sum to float ----
            float rowSumFloat = static_cast<float>(rowAccumulator);

            // ---- Step 3: divide each entry by the row sum ----
            colIdx = 0;
            while (colIdx < matrixSize) {
                // Temporary conversion of the integer cell to float
                float cellValueFloat = static_cast<float>(rawAdj[rowIdx][colIdx]);
                // Normalized value stored in a temporary variable first
                float normalizedTmp = cellValueFloat / rowSumFloat;
                normAdj[rowIdx][colIdx] = normalizedTmp;
                ++colIdx;
            }
            ++rowIdx;
        }
    }

    // Print both raw and normalized matrices
    void displayResults() const {
        std::cout << "Raw adjacency matrix (int):\n";
        for (int i = 0; i < matrixSize; ++i) {
            for (int j = 0; j < matrixSize; ++j) {
                std::cout << rawAdj[i][j] << '\t';
            }
            std::cout << '\n';
        }

        std::cout << "\nNormalized adjacency matrix (float, rows sum to 1):\n";
        for (int i = 0; i < matrixSize; ++i) {
            for (int j = 0; j < matrixSize; ++j) {
                // Show with limited precision for readability
                std::cout << static_cast<float>(static_cast<int>(normAdj[i][j] * 1000)) / 1000 << '\t';
            }
            std::cout << '\n';
        }
    }

private:
    int matrixSize;          // Number of vertices (size of the square matrix)
    int** rawAdj;            // Original adjacency matrix (integer weights)
    float** normAdj;         // Normalized adjacency matrix (float values)
};

int main() {
    // Choose a "large" size; adjust as needed for demonstration
    const int largeSize = 20;   // 20 × 20 matrix

    // Create the normalizer object
    MatrixNormalizer normalizer(largeSize);

    // Step‑by‑step execution
    normalizer.generateRandomData();   // Fill matrix with deterministic sparse‑skewed data
    normalizer.normalizeRows();        // Row‑wise normalization
    normalizer.displayResults();       // Output both matrices

    return 0;
}
