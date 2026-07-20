#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

int main() {
    // ------------------------------------------------------------
    // 1. Define the size of the graph (number of vertices)
    // ------------------------------------------------------------
    int vertexCount = 8;

    // ------------------------------------------------------------
    // 2. Create a deterministic adjacency matrix (undirected graph)
    //    Sparse structure with a small cluster and isolated vertices
    // ------------------------------------------------------------
    std::vector<std::vector<float>> adjacency = {
        {0, 1, 1, 0, 0, 0, 0, 0}, // vertex 0 connected to 1 and 2
        {1, 0, 0, 0, 0, 0, 0, 0}, // vertex 1 connected to 0
        {1, 0, 0, 0, 0, 0, 0, 0}, // vertex 2 connected to 0
        {0, 0, 0, 0, 0, 0, 0, 0}, // vertex 3 isolated
        {0, 0, 0, 0, 0, 0, 0, 0}, // vertex 4 isolated
        {0, 0, 0, 0, 0, 0, 1, 0}, // vertex 5 connected to 6
        {0, 0, 0, 0, 0, 1, 0, 0}, // vertex 6 connected to 5
        {0, 0, 0, 0, 0, 0, 0, 0}  // vertex 7 isolated
    };

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
