#include <iostream>
#include <cmath>

/* LLM input variant 2: small-diverse */

int main() {
    // ------------------------------------------------------------
    // 1. Define graph size and allocate adjacency matrix on the heap
    // ------------------------------------------------------------
    int vertexCount = 4;
    int totalSize = vertexCount * vertexCount;
    float* adjacency = new float[totalSize];

    // ------------------------------------------------------------
    // 2. Fill adjacency matrix with a deterministic test graph
    //    (a 4‑node graph with varied edge weights and a self‑loop)
    // ------------------------------------------------------------
    for (int i = 0; i < totalSize; ++i) {
        adjacency[i] = 0.0f;                     // initialise all entries to zero
    }
    // edges are undirected, with diverse weights
    adjacency[0 * vertexCount + 1] = 2.5f;
    adjacency[1 * vertexCount + 0] = 2.5f;

    adjacency[0 * vertexCount + 2] = 0.7f;
    adjacency[2 * vertexCount + 0] = 0.7f;

    adjacency[1 * vertexCount + 2] = 1.3f;
    adjacency[2 * vertexCount + 1] = 1.3f;

    adjacency[2 * vertexCount + 3] = 2.0f;
    adjacency[3 * vertexCount + 2] = 2.0f;

    adjacency[3 * vertexCount + 0] = 0.8f;
    adjacency[0 * vertexCount + 3] = 0.8f;

    // add a small self‑loop at vertex 1
    adjacency[1 * vertexCount + 1] = 0.3f;

    // ------------------------------------------------------------
    // 3. Allocate working matrices for the QR iteration
    // ------------------------------------------------------------
    float* matrixA = new float[totalSize];   // copy of adjacency, will be updated
    float* matrixQ = new float[totalSize];   // orthogonal factor
    float* matrixR = new float[totalSize];   // upper‑triangular factor

    // copy adjacency into matrixA
    for (int i = 0; i < totalSize; ++i) {
        matrixA[i] = adjacency[i];
    }

    // ------------------------------------------------------------
    // 4. QR iteration parameters
    // ------------------------------------------------------------
    int maxIter = 120;          // number of QR steps
    float tolerance = 1e-5f;    // not used for simplicity, fixed iterations

    // ------------------------------------------------------------
    // 5. Begin QR iterations (simple Gram‑Schmidt QR)
    // ------------------------------------------------------------
    for (int iter = 0; iter < maxIter; ++iter) {
        // ---- 5.1 Initialise Q and R to zero ----
        for (int i = 0; i < totalSize; ++i) {
            matrixQ[i] = 0.0f;
            matrixR[i] = 0.0f;
        }

        // ---- 5.2 Classical Gram‑Schmidt ----
        // Columns of A are processed one by one
        for (int col = 0; col < vertexCount; ++col) {
            // copy current column of A into temporary vector v
            float* v = new float[vertexCount];
            for (int row = 0; row < vertexCount; ++row) {
                v[row] = matrixA[row * vertexCount + col];
            }

            // orthogonalise against previous Q columns
            for (int prev = 0; prev < col; ++prev) {
                // compute dot product q_prev · v
                float dotProd = 0.0f;
                for (int k = 0; k < vertexCount; ++k) {
                    dotProd += matrixQ[k * vertexCount + prev] * v[k];
                }
                // store in R[prev][col]
                matrixR[prev * vertexCount + col] = dotProd;
                // subtract projection
                for (int k = 0; k < vertexCount; ++k) {
                    v[k] -= dotProd * matrixQ[k * vertexCount + prev];
                }
            }

            // compute norm of the orthogonalised vector
            float normVal = 0.0f;
            for (int k = 0; k < vertexCount; ++k) {
                normVal += v[k] * v[k];
            }
            normVal = std::sqrt(normVal);

            // store norm in R[col][col]
            matrixR[col * vertexCount + col] = normVal;

            // normalise to obtain Q column
            for (int k = 0; k < vertexCount; ++k) {
                if (normVal != 0.0f) {
                    matrixQ[k * vertexCount + col] = v[k] / normVal;
                } else {
                    matrixQ[k * vertexCount + col] = 0.0f;
                }
            }

            delete[] v; // free temporary vector
        }

        // ---- 5.3 Form the next A = R * Q ----
        // temporary matrix to hold the product
        float* nextA = new float[totalSize];
        for (int i = 0; i < totalSize; ++i) {
            nextA[i] = 0.0f;
        }

        // matrix multiplication: nextA = R * Q
        for (int i = 0; i < vertexCount; ++i) {
            for (int j = 0; j < vertexCount; ++j) {
                float sumTmp = 0.0f;
                for (int k = 0; k < vertexCount; ++k) {
                    sumTmp += matrixR[i * vertexCount + k] * matrixQ[k * vertexCount + j];
                }
                nextA[i * vertexCount + j] = sumTmp;
            }
        }

        // copy back into matrixA for the next iteration
        for (int i = 0; i < totalSize; ++i) {
            matrixA[i] = nextA[i];
        }

        delete[] nextA; // free the temporary product matrix
    }

    // ------------------------------------------------------------
    // 6. Extract the diagonal of the final matrixA – these are the eigenvalues
    // ------------------------------------------------------------
    std::cout << "Adjacency spectrum (approximate eigenvalues):\n";
    for (int i = 0; i < vertexCount; ++i) {
        float eigenVal = matrixA[i * vertexCount + i];
        std::cout << "  λ[" << i << "] = " << eigenVal << "\n";
    }

    // ------------------------------------------------------------
    // 7. Clean up heap memory
    // ------------------------------------------------------------
    delete[] adjacency;
    delete[] matrixA;
    delete[] matrixQ;
    delete[] matrixR;

    return 0;
}
