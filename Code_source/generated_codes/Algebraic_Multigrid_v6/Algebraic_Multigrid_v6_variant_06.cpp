#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 6: ordered-structured */

int main() {
    // -------------------------------------------------
    // 1) Problem definition (1‑D Poisson, larger structured input)
    // -------------------------------------------------
    int szFine = 9; // increased size for a more structured dataset
    std::vector<std::vector<float>> matA(szFine, std::vector<float>(szFine, 0.0f));
    std::vector<float> vecb(szFine, 0.0f);
    std::vector<float> solX(szFine, 0.0f);

    // Fill A with the 5‑point 1‑D Laplacian (still symmetric)
    for (int i = 0; i < szFine; ++i) {
        for (int j = 0; j < szFine; ++j) {
            if (i == j) {
                matA[i][j] = 2.0f;
            } else if (std::abs(i - j) == 1) {
                matA[i][j] = -1.0f;
            } else {
                matA[i][j] = 0.0f;
            }
        }
    }

    // Right‑hand side: symmetric structured vector [1,2,3,4,5,4,3,2,1]^T
    for (int i = 0; i < szFine; ++i) {
        if (i <= szFine / 2) {
            vecb[i] = static_cast<float>(i + 1);
        } else {
            vecb[i] = static_cast<float>(szFine - i);
        }
    }

    // -------------------------------------------------
    // 2) Helper lambdas (matrix‑vector, smoothing, etc.)
    // -------------------------------------------------
    auto mulMatVec = [&](const std::vector<std::vector<float>>& M,
                         const std::vector<float>& v,
                         std::vector<float>& out) {
        int n = static_cast<int>(M.size());
        for (int i = 0; i < n; ++i) {
            float acc = 0.0f;
            for (int j = 0; j < n; ++j) {
                acc += M[i][j] * v[j];
            }
            out[i] = acc;
        }
    };

    // Gauss‑Seidel smoother (in‑place)
    auto gaussSeidel = [&](const std::vector<std::vector<float>>& M,
                           const std::vector<float>& rhs,
                           std::vector<float>& x,
                           int sweeps) {
        int n = static_cast<int>(M.size());
        for (int s = 0; s < sweeps; ++s) {
            for (int i = 0; i < n; ++i) {
                float sigma = 0.0f;
                for (int j = 0; j < n; ++j) {
                    if (j != i) sigma += M[i][j] * x[j];
                }
                x[i] = (rhs[i] - sigma) / M[i][i];
            }
        }
    };

    // Restriction operator (injection)
    auto restrictVec = [&](const std::vector<float>& fine,
                           std::vector<float>& coarse) {
        int nC = static_cast<int>(coarse.size());
        for (int i = 0; i < nC; ++i) {
            coarse[i] = fine[2 * i];
        }
    };

    // Prolongation operator (linear interpolation)
    auto prolongateVec = [&](const std::vector<float>& coarse,
                            std::vector<float>& fine) {
        int nC = static_cast<int>(coarse.size());
        int nF = static_cast<int>(fine.size());
        for (int i = 0; i < nF; ++i) {
            if (i % 2 == 0) {
                fine[i] = coarse[i / 2];
            } else {
                int left = i / 2;
                int right = left + 1;
                if (right >= nC) right = left;        // boundary handling
                fine[i] = 0.5f * (coarse[left] + coarse[right]);
            }
        }
    };

    // -------------------------------------------------
    // 3) Two‑level AMG V‑cycle (loop‑heavy, everything inside main)
    // -------------------------------------------------
    int preSmooth = 2;
    int postSmooth = 2;
    int coarseSweeps = 10;                       // solve coarse system iteratively

    // Pre‑smoothing
    gaussSeidel(matA, vecb, solX, preSmooth);

    // Compute residual r = b – A·x
    std::vector<float> resid(szFine, 0.0f);
    std::vector<float> Ax(szFine, 0.0f);
    mulMatVec(matA, solX, Ax);
    for (int i = 0; i < szFine; ++i) {
        resid[i] = vecb[i] - Ax[i];
    }

    // Restrict residual to coarse grid
    int szCoarse = (szFine + 1) / 2; // number of injected points
    std::vector<float> residC(szCoarse, 0.0f);
    restrictVec(resid, residC);

    // Build coarse matrix A_c = R·A·P (here R = injection, P = linear)
    std::vector<std::vector<float>> matAc(szCoarse, std::vector<float>(szCoarse, 0.0f));
    // Assemble A_c entry‑wise (loop‑heavy)
    for (int i = 0; i < szCoarse; ++i) {
        for (int j = 0; j < szCoarse; ++j) {
            float acc = 0.0f;
            // contribution from fine points 2*i and 2*j
            for (int p = 0; p < szFine; ++p) {
                float rp = (p == 2 * i) ? 1.0f : 0.0f;          // restriction weight
                for (int q = 0; q < szFine; ++q) {
                    float pq = 0.0f;                           // prolongation weight
                    if (q == 2 * j) pq = 1.0f;
                    else if (q == 2 * j - 1) pq = 0.5f;
                    else if (q == 2 * j + 1) pq = 0.5f;
                    acc += rp * matA[p][q] * pq;
                }
            }
            matAc[i][j] = acc;
        }
    }

    // Solve coarse system A_c·e_c = r_c (few Gauss‑Seidel sweeps)
    std::vector<float> errC(szCoarse, 0.0f);
    gaussSeidel(matAc, residC, errC, coarseSweeps);

    // Prolongate error to fine grid and correct solution
    std::vector<float> errF(szFine, 0.0f);
    prolongateVec(errC, errF);
    for (int i = 0; i < szFine; ++i) {
        solX[i] += errF[i];
    }

    // Post‑smoothing
    gaussSeidel(matA, vecb, solX, postSmooth);

    // -------------------------------------------------
    // 4) Output the final approximation
    // -------------------------------------------------
    std::cout << "Approximate solution (AMG V‑cycle) for structured 9‑point Poisson:\n";
    for (int i = 0; i < szFine; ++i) {
        std::cout << "x[" << i << "] = " << solX[i] << "\n";
    }

    return 0;
}
