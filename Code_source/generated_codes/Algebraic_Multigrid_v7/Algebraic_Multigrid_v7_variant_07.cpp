// Algebraic Multigrid implementation – Version #7
/* LLM input variant 7: reverse-adversarial */
// ------------------------------------------------
// This program builds a tiny 1‑D Poisson problem (5 unknowns)
// and solves it with a two‑level AMG V‑cycle.
// Only float and int are used, all data live on the stack,
// and the code follows a verbose, step‑by‑step style.

#include <iostream>
#include <iomanip>

// -----------------------------------------------------------------
// Problem dimensions (stack‑allocated arrays)
// -----------------------------------------------------------------
#define NF 5   // number of fine‑grid points
#define NC 3   // number of coarse‑grid points

// -----------------------------------------------------------------
// Simple class that contains the whole AMG machinery
// -----------------------------------------------------------------
class AlgebraicMultigrid {
public:
    // -----------------------------------------------------------------
    // Matrices and vectors (all stored as plain C‑style arrays)
    // -----------------------------------------------------------------
    float matA[NF][NF];       // fine‑grid operator
    float matR[NC][NF];       // restriction operator
    float matP[NF][NC];       // prolongation operator (transpose of R)
    float matAc[NC][NC];      // coarse‑grid operator (R * A * P)

    float vecX[NF];           // current fine‑grid solution
    float vecB[NF];           // fine‑grid right‑hand side

    float vecRc[NC];          // coarse‑grid right‑hand side (restricted residual)
    float vecXc[NC];          // coarse‑grid solution

    // -----------------------------------------------------------------
    // Constructor – builds matrices and sets up the test problem
    // -----------------------------------------------------------------
    AlgebraicMultigrid() {
        build_fine_operator();      // A = 2 on diagonal, -1 on neighbours
        build_transfer_operators(); // R = reversed injection, P = transpose(R)
        build_coarse_operator();    // Ac = R * A * P
        initialise_vectors();       // zero initial guess, descending RHS
    }

    // -----------------------------------------------------------------
    // Public entry point – run several V‑cycles and display the result
    // -----------------------------------------------------------------
    void run() {
        int cycle = 0;
        while (cycle < 8) {               // eight V‑cycles
            std::cout << "=== V‑cycle " << (cycle+1) << " ===\n";
            v_cycle();                    // one two‑level V‑cycle
            ++cycle;
        }
        print_solution();
    }

private:
    // -----------------------------------------------------------------
    // Build the 5‑point 1‑D Laplacian: 2 on the diagonal, -1 on neighbours
    // -----------------------------------------------------------------
    void build_fine_operator() {
        int i = 0;
        while (i < NF) {
            int j = 0;
            while (j < NF) {
                if (i == j) {
                    matA[i][j] = 2.0f;
                } else if (i == j+1 || i+1 == j) {
                    matA[i][j] = -1.0f;
                } else {
                    matA[i][j] = 0.0f;
                }
                ++j;
            }
            ++i;
        }
    }

    // -----------------------------------------------------------------
    // Restriction: reversed injection (coarse point i gets fine point NF-1-2*i)
    // Prolongation: transpose of restriction
    // -----------------------------------------------------------------
    void build_transfer_operators() {
        // Zero everything first
        int i = 0;
        while (i < NC) {
            int j = 0;
            while (j < NF) {
                matR[i][j] = 0.0f;
                ++j;
            }
            ++i;
        }

        // Reversed injection pattern
        i = 0;
        while (i < NC) {
            int fineIdx = NF - 1 - i * 2;   // 4,2,0 for i=0,1,2
            matR[i][fineIdx] = 1.0f;
            ++i;
        }

        // Prolongation is the transpose of R
        i = 0;
        while (i < NF) {
            int j = 0;
            while (j < NC) {
                matP[i][j] = matR[j][i];
                ++j;
            }
            ++i;
        }
    }

    // -----------------------------------------------------------------
    // Compute coarse operator Ac = R * A * P
    // -----------------------------------------------------------------
    void build_coarse_operator() {
        int i = 0;
        while (i < NC) {
            int j = 0;
            while (j < NC) {
                float sum = 0.0f;
                int k = 0;
                while (k < NF) {
                    int l = 0;
                    while (l < NF) {
                        sum += matR[i][k] * matA[k][l] * matP[l][j];
                        ++l;
                    }
                    ++k;
                }
                matAc[i][j] = sum;
                ++j;
            }
            ++i;
        }
    }

    // -----------------------------------------------------------------
    // Initialise solution vector (zero) and a descending RHS (e.g., 5,4,3,2,1)
    // -----------------------------------------------------------------
    void initialise_vectors() {
        int i = 0;
        while (i < NF) {
            vecX[i] = 0.0f;
            vecB[i] = static_cast<float>(NF - i); // descending source term
            ++i;
        }
        i = 0;
        while (i < NC) {
            vecXc[i] = 0.0f;
            ++i;
        }
    }

    // -----------------------------------------------------------------
    // One weighted Jacobi smoothing sweep on the fine grid
    // -----------------------------------------------------------------
    void smooth_fine(int sweeps) {
        const float omega = 0.7f;   // relaxation weight
        int s = 0;
        while (s < sweeps) {
            float newX[NF];
            int i = 0;
            while (i < NF) {
                float sigma = 0.0f;
                int j = 0;
                while (j < NF) {
                    if (j != i) {
                        sigma += matA[i][j] * vecX[j];
                    }
                    ++j;
                }
                // Jacobi update
                newX[i] = (1.0f - omega) * vecX[i] + omega * (vecB[i] - sigma) / matA[i][i];
                ++i;
            }
            // Copy back
            i = 0;
            while (i < NF) {
                vecX[i] = newX[i];
                ++i;
            }
            ++s;
        }
    }

    // -----------------------------------------------------------------
    // Compute residual r = b - A * x (fine grid)
    // -----------------------------------------------------------------
    void compute_residual_fine(float res[]) {
        int i = 0;
        while (i < NF) {
            float Ax = 0.0f;
            int j = 0;
            while (j < NF) {
                Ax += matA[i][j] * vecX[j];
                ++j;
            }
            res[i] = vecB[i] - Ax;
            ++i;
        }
    }

    // -----------------------------------------------------------------
    // Restrict residual from fine to coarse: rc = R * r
    // -----------------------------------------------------------------
    void restrict_residual(const float fineRes[], float coarseRes[]) {
        int i = 0;
        while (i < NC) {
            float sum = 0.0f;
            int j = 0;
            while (j < NF) {
                sum += matR[i][j] * fineRes[j];
                ++j;
            }
            coarseRes[i] = sum;
            ++i;
        }
    }

    // -----------------------------------------------------------------
    // One weighted Jacobi smoothing sweep on the coarse grid
    // -----------------------------------------------------------------
    void smooth_coarse(int sweeps) {
        const float omega = 0.7f;
        int s = 0;
        while (s < sweeps) {
            float newXc[NC];
            int i = 0;
            while (i < NC) {
                float sigma = 0.0f;
                int j = 0;
                while (j < NC) {
                    if (j != i) {
                        sigma += matAc[i][j] * vecXc[j];
                    }
                    ++j;
                }
                newXc[i] = (1.0f - omega) * vecXc[i] + omega * (vecRc[i] - sigma) / matAc[i][i];
                ++i;
            }
            i = 0;
            while (i < NC) {
                vecXc[i] = newXc[i];
                ++i;
            }
            ++s;
        }
    }

    // -----------------------------------------------------------------
    // Prolongate coarse correction and add to fine solution: x += P * ec
    // -----------------------------------------------------------------
    void prolongate_and_correct() {
        float corr[NF];
        int i = 0;
        while (i < NF) {
            float sum = 0.0f;
            int j = 0;
            while (j < NC) {
                sum += matP[i][j] * vecXc[j];
                ++j;
            }
            corr[i] = sum;
            ++i;
        }
        // Apply correction
        i = 0;
        while (i < NF) {
            vecX[i] += corr[i];
            ++i;
        }
    }

    // -----------------------------------------------------------------
    // A single V‑cycle: pre‑smooth, restrict, coarse solve, prolongate, post‑smooth
    // -----------------------------------------------------------------
    void v_cycle() {
        // 1) Pre‑smoothing on fine grid
        smooth_fine(3);

        // 2) Compute fine‑grid residual
        float fineRes[NF];
        compute_residual_fine(fineRes);

        // 3) Restrict residual to coarse grid (right‑hand side for coarse problem)
        restrict_residual(fineRes, vecRc);

        // 4) Initialise coarse solution to zero (for clarity)
        int i = 0;
        while (i < NC) {
            vecXc[i] = 0.0f;
            ++i;
        }

        // 5) Solve coarse problem (few Jacobi sweeps)
        smooth_coarse(5);

        // 6) Prolongate coarse correction and update fine solution
        prolongate_and_correct();

        // 7) Post‑smoothing on fine grid
        smooth_fine(3);
    }

    // -----------------------------------------------------------------
    // Print the final fine‑grid solution
    // -----------------------------------------------------------------
    void print_solution() {
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "\nFinal fine‑grid solution (x):\n";
        int i = 0;
        while (i < NF) {
            std::cout << "x[" << i << "] = " << vecX[i] << "\n";
            ++i;
        }
    }
};

// -----------------------------------------------------------------
// Main entry point – create AMG object and run the solver
// -----------------------------------------------------------------
int main() {
    AlgebraicMultigrid solver;
    solver.run();
    return 0;
}
