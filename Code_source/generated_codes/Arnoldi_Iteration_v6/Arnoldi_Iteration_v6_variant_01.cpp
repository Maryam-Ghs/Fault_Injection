/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    // -------------------------------------------------
    // deterministic test data (minimal non‑trivial)
    // -------------------------------------------------
    int nDim = 2;                 // matrix dimension
    int mSize = 1;                // number of Arnoldi steps (≤ nDim)

    // Simple 2×2 upper‑triangular matrix with the smallest positive entries
    std::vector<std::vector<float>> matA(nDim, std::vector<float>(nDim, 0.0f));
    for (int i = 0; i < nDim; ++i) {
        for (int j = i; j < nDim; ++j) {
            matA[i][j] = 1.0f;                 // all ones on/above diagonal
        }
    }

    // initial vector (minimal size, non‑zero)
    std::vector<float> vecB(nDim, 0.0f);
    for (int i = 0; i < nDim; ++i) {
        vecB[i] = 1.0f;                         // [1, 1]
    }

    // -------------------------------------------------
    // storage for Arnoldi basis Q (nDim × (mSize+1))
    // and Hessenberg matrix H ((mSize+1) × mSize)
    // -------------------------------------------------
    std::vector<std::vector<float>> basis(nDim, std::vector<float>(mSize + 1, 0.0f));
    std::vector<std::vector<float>> hMat(mSize + 1, std::vector<float>(mSize, 0.0f));

    // -------------------------------------------------
    // normalize the first vector
    // -------------------------------------------------
    float norm0 = 0.0f;
    for (int i = 0; i < nDim; ++i) norm0 += vecB[i] * vecB[i];
    norm0 = sqrtf(norm0);
    for (int i = 0; i < nDim; ++i) basis[i][0] = vecB[i] / norm0;

    // -------------------------------------------------
    // Arnoldi loop (using while instead of for)
    // -------------------------------------------------
    int step = 0;
    while (step < mSize) {
        // w = A * q_k
        std::vector<float> temp(nDim, 0.0f);
        for (int row = 0; row < nDim; ++row) {
            float acc = 0.0f;
            for (int col = 0; col < nDim; ++col) {
                acc += matA[row][col] * basis[col][step];
            }
            temp[row] = acc;                     // store w temporarily
        }

        // Orthogonalisation against previous basis vectors
        int inner = 0;
        while (inner <= step) {
            // h_ij = q_i^T * w
            float dot = 0.0f;
            for (int idx = 0; idx < nDim; ++idx) dot += basis[idx][inner] * temp[idx];
            hMat[inner][step] = dot;

            // w = w - h_ij * q_i   (reordered arithmetic)
            for (int idx = 0; idx < nDim; ++idx) temp[idx] -= dot * basis[idx][inner];
            ++inner;
        }

        // h_{k+1,k} = ||w||
        float newNorm = 0.0f;
        for (int i = 0; i < nDim; ++i) newNorm += temp[i] * temp[i];
        newNorm = sqrtf(newNorm);
        hMat[step + 1][step] = newNorm;

        // stop if the vector vanished
        if (newNorm == 0.0f) break;

        // q_{k+1} = w / h_{k+1,k}
        for (int i = 0; i < nDim; ++i) basis[i][step + 1] = temp[i] / newNorm;

        ++step;
    }

    // -------------------------------------------------
    // output the computed orthonormal basis Q
    // -------------------------------------------------
    std::cout << "Orthonormal basis Q (columns):\n";
    for (int i = 0; i < nDim; ++i) {
        for (int j = 0; j <= step; ++j) {
            std::cout << basis[i][j] << ' ';
        }
        std::cout << '\n';
    }

    // -------------------------------------------------
    // output the Hessenberg matrix H
    // -------------------------------------------------
    std::cout << "\nUpper Hessenberg matrix H:\n";
    for (int i = 0; i <= step; ++i) {
        for (int j = 0; j < step; ++j) {
            if (i > j + 1) std::cout << "0 ";
            else std::cout << hMat[i][j] << ' ';
        }
        std::cout << '\n';
    }

    return 0;
}
