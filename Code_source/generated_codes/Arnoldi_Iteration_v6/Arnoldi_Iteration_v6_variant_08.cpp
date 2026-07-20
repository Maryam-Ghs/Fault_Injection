/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    // -------------------------------------------------
    // deterministic test data (sparse, skewed)
    // -------------------------------------------------
    int nDim = 6;
    int mSize = 5;

    // Upper‑triangular sparse matrix: only diagonal and last column are 1, rest 0
    std::vector<std::vector<float>> matA(nDim, std::vector<float>(nDim, 0.0f));
    for (int i = 0; i < nDim; ++i) {
        for (int j = i; j < nDim; ++j) {
            if (i == j || j == nDim - 1) {
                matA[i][j] = 1.0f;
            }
        }
    }

    // initial vector mostly zeros, single non‑zero entry
    std::vector<float> vecB(nDim, 0.0f);
    vecB[0] = 1.0f;                               // only first component non‑zero

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
    // Arnoldi loop
    // -------------------------------------------------
    int step = 0;
    while (step < mSize) {
        std::vector<float> temp(nDim, 0.0f);
        for (int row = 0; row < nDim; ++row) {
            float acc = 0.0f;
            for (int col = 0; col < nDim; ++col) {
                acc += matA[row][col] * basis[col][step];
            }
            temp[row] = acc;
        }

        int inner = 0;
        while (inner <= step) {
            float dot = 0.0f;
            for (int idx = 0; idx < nDim; ++idx) dot += basis[idx][inner] * temp[idx];
            hMat[inner][step] = dot;
            for (int idx = 0; idx < nDim; ++idx) temp[idx] -= dot * basis[idx][inner];
            ++inner;
        }

        float newNorm = 0.0f;
        for (int i = 0; i < nDim; ++i) newNorm += temp[i] * temp[i];
        newNorm = sqrtf(newNorm);
        hMat[step + 1][step] = newNorm;
        if (newNorm == 0.0f) break;
        for (int i = 0; i < nDim; ++i) basis[i][step + 1] = temp[i] / newNorm;
        ++step;
    }

    // -------------------------------------------------
    // output Q
    // -------------------------------------------------
    std::cout << "Orthonormal basis Q (columns):\n";
    for (int i = 0; i < nDim; ++i) {
        for (int j = 0; j <= step; ++j) {
            std::cout << basis[i][j] << ' ';
        }
        std::cout << '\n';
    }

    // -------------------------------------------------
    // output H
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
