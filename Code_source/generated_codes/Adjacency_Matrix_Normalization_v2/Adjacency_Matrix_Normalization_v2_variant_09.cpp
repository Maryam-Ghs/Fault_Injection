#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // ------------------------------------------------------------
    // 1. Define the size of the graph (number of vertices)
    // ------------------------------------------------------------
    int vertexCount = 7;

    // ------------------------------------------------------------
    // 2. Create a deterministic adjacency matrix (undirected graph)
    //    (pseudo‑random looking weights, symmetric, zero diagonal)
    // ------------------------------------------------------------
    std::vector<std::vector<float>> adjacency = {
        {0.0f, 1.5f, 0.0f, 0.7f, 0.0f, 0.0f, 1.2f},
        {1.5f, 0.0f, 2.0f, 0.0f, 0.8f, 0.0f, 0.0f},
        {0.0f, 2.0f, 0.0f, 1.1f, 0.0f, 0.9f, 0.0f},
        {0.7f, 0.0f, 1.1f, 0.0f, 2.3f, 0.0f, 0.0f},
        {0.0f, 0.8f, 0.0f, 2.3f, 0.0f, 1.4f, 0.0f},
        {0.0f, 0.0f, 0.9f, 0.0f, 1.4f, 0.0f, 2.2f},
        {1.2f, 0.0f, 0.0f, 0.0f, 0.0f, 2.2f, 0.0f}
    };

    // ------------------------------------------------------------
    // 3. Compute the degree of each vertex (row sum of adjacency)
    // ------------------------------------------------------------
    std::vector<float> degree(vertexCount, 0.0f);
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
                                               std::vector<float>(vertexCount, 0.0f));

    // ------------------------------------------------------------
    // 5. Fill the normalized matrix using the formula:
    //    N[i][j] = A[i][j] / sqrt( degree[i] * degree[j] )
    //    (skip division when either degree is zero)
    // ------------------------------------------------------------
    row = 0;
    while (row < vertexCount) {
        int col = 0;
        while (col < vertexCount) {
            if (degree[row] > 0.0f && degree[col] > 0.0f) {
                float denominator = sqrtf(degree[row] * degree[col]);
                normalized[row][col] = adjacency[row][col] / denominator;
            } else {
                normalized[row][col] = 0.0f;
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
