#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

int main() {
    // ------------------------------------------------------------
    // 1. Define the size of the graph (number of vertices)
    // ------------------------------------------------------------
    int vertexCount = 200; // larger but memory‑safe size

    // ------------------------------------------------------------
    // 2. Create a deterministic adjacency matrix (undirected chain graph)
    //    Each vertex i (0 ≤ i < vertexCount‑1) is connected to i+1.
    // ------------------------------------------------------------
    std::vector<std::vector<float>> adjacency(vertexCount,
                                              std::vector<float>(vertexCount, 0.0f));
    for (int i = 0; i < vertexCount - 1; ++i) {
        adjacency[i][i + 1] = 1.0f;
        adjacency[i + 1][i] = 1.0f;
    }

    // ------------------------------------------------------------
    // 3. Compute the degree of each vertex (row sum of adjacency)
    // ------------------------------------------------------------
    std::vector<float> degree(vertexCount, 0);
    int row = 0;
    while (row < vertexCount) {
        int col = 0;
        while (col < vertexCount) {
            degree[row] = degree[row] + adjacency[row][col];
            ++col;
        }
        ++row;
    }

    // ------------------------------------------------------------
    // 4. Allocate storage for the normalized adjacency matrix
    // ------------------------------------------------------------
    std::vector<std::vector<float>> normalized(vertexCount,
                                               std::vector<float>(vertexCount, 0));

    // ------------------------------------------------------------
    // 5. Fill the normalized matrix using the formula:
    //    N[i][j] = A[i][j] / sqrt( degree[i] * degree[j] )
    //    (skip division when either degree is zero)
    // ------------------------------------------------------------
    row = 0;
    while (row < vertexCount) {
        int col = 0;
        while (col < vertexCount) {
            if (degree[row] > 0 && degree[col] > 0) {
                float denominator = sqrtf(degree[row] * degree[col]);
                normalized[row][col] = adjacency[row][col] / denominator;
            } else {
                normalized[row][col] = 0;
            }
            ++col;
        }
        ++row;
    }

    // ------------------------------------------------------------
    // 6. Print the resulting normalized adjacency matrix
    // ------------------------------------------------------------
    std::cout << "Normalized adjacency matrix (version #2):\n";
    row = 0;
    while (row < vertexCount) {
        int col = 0;
        while (col < vertexCount) {
            std::cout << std::fixed << std::setprecision(3) << normalized[row][col];
            if (col < vertexCount - 1) std::cout << " ";
            ++col;
        }
        std::cout << "\n";
        ++row;
    }

    return 0;
}
