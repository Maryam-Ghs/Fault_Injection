/* LLM input variant 5: duplicate-heavy */

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

    // Populate raw adjacency matrix with deterministic duplicate‑heavy data
    void generateRandomData() {
        // Fill every cell with the same value to create many duplicates
        int rowIdx = 0;
        while (rowIdx < matrixSize) {
            int colIdx = 0;
            while (colIdx < matrixSize) {
                rawAdj[rowIdx][colIdx] = 3; // constant weight
                ++colIdx;
            }
            ++rowIdx;
        }

        // Guarantee each row has a non‑zero sum (avoid division by zero)
        // (Redundant here because all entries are 3, but kept for safety)
        for (int i = 0; i < matrixSize; ++i) {
            int sumCheck = 0;
            for (int j = 0; j < matrixSize; ++j) {
                sumCheck += rawAdj[i][j];
            }
            if (sumCheck == 0) {
                // Force the diagonal entry to 1
                rawAdj[i][i] = 1;
            }
        }
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
    normalizer.generateRandomData();   // Fill matrix with duplicate‑heavy ints
    normalizer.normalizeRows();        // Row‑wise normalization
    normalizer.displayResults();       // Output both matrices

    return 0;
}
