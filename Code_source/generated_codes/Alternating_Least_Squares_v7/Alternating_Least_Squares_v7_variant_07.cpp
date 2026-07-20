/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <vector>
#include <cmath>

// ------------------------------------------------------------
// Alternating Least Squares (ALS) implementation – version #7
// ------------------------------------------------------------
// This program factorises a deterministic rating matrix R into
// two low‑rank matrices U (users × factors) and V (items × factors)
// using the ALS optimisation scheme.  All arithmetic is performed
// with the float type only, and the code is written in a verbose,
// step‑by‑step style.  The implementation is class based and makes
// extensive use of std::vector.
//
// ------------------------------------------------------------

class AlsEngine {
public:
    // --------------------------------------------------------
    // Public interface
    // --------------------------------------------------------
    AlsEngine(int userCnt, int itemCnt, int factorCnt, int iters, float lambda);
    void run();                     // execute ALS
    void showResult() const;        // print U and V matrices

private:
    // --------------------------------------------------------
    // Data members
    // --------------------------------------------------------
    int userNum;                     // number of users
    int itemNum;                     // number of items
    int factorNum;                   // latent dimension
    int maxIter;                     // ALS iterations
    float reg;                       // regularisation term

    std::vector< std::vector<float> > matR;   // rating matrix (user × item)
    std::vector< std::vector<float> > matU;   // user factors (user × factor)
    std::vector< std::vector<float> > matV;   // item factors (item × factor)

    // --------------------------------------------------------
    // Helper methods
    // --------------------------------------------------------
    void initFactors();                              // deterministic initialisation
    void updateUsers();                              // solve for U given V
    void updateItems();                              // solve for V given U
    void solveLinear2x2(const std::vector<float>& A, const std::vector<float>& b,
                        std::vector<float>& x) const; // 2×2 linear solver
    void buildRatingMatrix();                        // deterministic test data
};

// ------------------------------------------------------------
// Constructor – initialise dimensions and allocate storage
// ------------------------------------------------------------
AlsEngine::AlsEngine(int userCnt, int itemCnt, int factorCnt, int iters, float lambda)
    : userNum(userCnt), itemNum(itemCnt), factorNum(factorCnt),
      maxIter(iters), reg(lambda)
{
    // Allocate rating matrix
    matR.assign(userNum, std::vector<float>(itemNum, 0.0f));

    // Allocate factor matrices
    matU.assign(userNum, std::vector<float>(factorNum, 0.0f));
    matV.assign(itemNum, std::vector<float>(factorNum, 0.0f));

    // Fill rating matrix with deterministic reverse‑ordered values
    buildRatingMatrix();

    // Initialise factor matrices with a deterministic reverse pattern
    initFactors();
}

// ------------------------------------------------------------
// Fill matR with a reverse‑ordered dense test matrix
// ------------------------------------------------------------
void AlsEngine::buildRatingMatrix()
{
    // Create a dense matrix where values decrease when moving right or down
    // Example for 5 users, 4 items:
    // 20 15 10 5
    // 16 12 8 4
    // 12 9 6 3
    // 8 6 4 2
    // 4 3 2 1
    for (int u = 0; u < userNum; ++u) {
        for (int i = 0; i < itemNum; ++i) {
            matR[u][i] = static_cast<float>((userNum - u) * (itemNum - i));
        }
    }
}

// ------------------------------------------------------------
// Initialise factor matrices with a reverse‑ordered deterministic pattern
// ------------------------------------------------------------
void AlsEngine::initFactors()
{
    // User factors: larger values for earlier users, decreasing with factor index
    for (int u = 0; u < userNum; ++u) {
        for (int f = 0; f < factorNum; ++f) {
            // (users - u) / (f + 1)
            matU[u][f] = static_cast<float>(userNum - u) / static_cast<float>(f + 1);
        }
    }

    // Item factors: larger values for earlier items, decreasing with factor index
    for (int i = 0; i < itemNum; ++i) {
        for (int f = 0; f < factorNum; ++f) {
            // (items - i) / (f + 2)
            matV[i][f] = static_cast<float>(itemNum - i) / static_cast<float>(f + 2);
        }
    }
}

// ------------------------------------------------------------
// Run the ALS optimisation loop
// ------------------------------------------------------------
void AlsEngine::run()
{
    int round = 0;
    while (round < maxIter) {
        std::cout << "=== ALS iteration " << round + 1 << " ===\n";

        // 1) Update user factors while keeping items fixed
        updateUsers();

        // 2) Update item factors while keeping users fixed
        updateItems();

        ++round;
    }
}

// ------------------------------------------------------------
// Update user factor matrix matU given current matV
// ------------------------------------------------------------
void AlsEngine::updateUsers()
{
    // Pre‑compute Vᵀ·V once because it is shared across users
    std::vector<float> vtV( factorNum * factorNum, 0.0f );
    for (int a = 0; a < factorNum; ++a) {
        for (int b = 0; b < factorNum; ++b) {
            float sum = 0.0f;
            for (int i = 0; i < itemNum; ++i) {
                sum = sum + matV[i][a] * matV[i][b];
            }
            vtV[a * factorNum + b] = sum + ((a == b) ? reg : 0.0f);
        }
    }

    // Solve for each user independently
    for (int u = 0; u < userNum; ++u) {
        // Build right‑hand side vector (Vᵀ·rᵤ)
        std::vector<float> rhs( factorNum, 0.0f );
        for (int f = 0; f < factorNum; ++f) {
            float acc = 0.0f;
            for (int i = 0; i < itemNum; ++i) {
                float rating = matR[u][i];
                // Skip missing entries (rating == 0)
                if (rating != 0.0f) {
                    acc = acc + matV[i][f] * rating;
                }
            }
            rhs[f] = acc;
        }

        // Solve the linear system: (Vᵀ·V + λI)·x = rhs
        solveLinear2x2(vtV, rhs, matU[u]);
    }
}

// ------------------------------------------------------------
// Update item factor matrix matV given current matU
// ------------------------------------------------------------
void AlsEngine::updateItems()
{
    // Pre‑compute Uᵀ·U once because it is shared across items
    std::vector<float> utU( factorNum * factorNum, 0.0f );
    for (int a = 0; a < factorNum; ++a) {
        for (int b = 0; b < factorNum; ++b) {
            float sum = 0.0f;
            for (int u = 0; u < userNum; ++u) {
                sum = sum + matU[u][a] * matU[u][b];
            }
            utU[a * factorNum + b] = sum + ((a == b) ? reg : 0.0f);
        }
    }

    // Solve for each item independently
    for (int i = 0; i < itemNum; ++i) {
        // Build right‑hand side vector (Uᵀ·rᵢ)
        std::vector<float> rhs( factorNum, 0.0f );
        for (int f = 0; f < factorNum; ++f) {
            float acc = 0.0f;
            for (int u = 0; u < userNum; ++u) {
                float rating = matR[u][i];
                // Skip missing entries
                if (rating != 0.0f) {
                    acc = acc + matU[u][f] * rating;
                }
            }
            rhs[f] = acc;
        }

        // Solve the linear system: (Uᵀ·U + λI)·x = rhs
        solveLinear2x2(utU, rhs, matV[i]);
    }
}

// ------------------------------------------------------------
// Solve a 2×2 linear system A·x = b  (A stored row‑major)
// ------------------------------------------------------------
void AlsEngine::solveLinear2x2(const std::vector<float>& A,
                               const std::vector<float>& b,
                               std::vector<float>& x) const
{
    // A = [ a00 a01
    //       a10 a11 ]
    float a00 = A[0];
    float a01 = A[1];
    float a10 = A[2];
    float a11 = A[3];

    // Compute determinant (reordered multiplication)
    float det = a00 * a11 - a01 * a10;

    // Guard against singular matrix (should not happen with regularisation)
    if (det == 0.0f) {
        x[0] = 0.0f;
        x[1] = 0.0f;
        return;
    }

    // Inverse of A (reordered signs)
    float inv00 =  a11 / det;
    float inv01 = -a01 / det;
    float inv10 = -a10 / det;
    float inv11 =  a00 / det;

    // x = A⁻¹·b  (reordered dot products)
    x[0] = inv00 * b[0] + inv01 * b[1];
    x[1] = inv10 * b[0] + inv11 * b[1];
}

// ------------------------------------------------------------
// Print the factor matrices U and V
// ------------------------------------------------------------
void AlsEngine::showResult() const
{
    std::cout << "\nUser factor matrix U (" << userNum << " × " << factorNum << "):\n";
    for (int u = 0; u < userNum; ++u) {
        for (int f = 0; f < factorNum; ++f) {
            std::cout << matU[u][f] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "\nItem factor matrix V (" << itemNum << " × " << factorNum << "):\n";
    for (int i = 0; i < itemNum; ++i) {
        for (int f = 0; f < factorNum; ++f) {
            std::cout << matV[i][f] << ' ';
        }
        std::cout << '\n';
    }

    // Optionally compute reconstruction error (verbose)
    std::cout << "\nReconstruction (U·Vᵀ):\n";
    for (int u = 0; u < userNum; ++u) {
        for (int i = 0; i < itemNum; ++i) {
            float pred = 0.0f;
            for (int f = 0; f < factorNum; ++f) {
                pred = pred + matU[u][f] * matV[i][f];
            }
            std::cout << pred << ' ';
        }
        std::cout << '\n';
    }
}

// ------------------------------------------------------------
// Main entry point – set up a reverse‑ordered deterministic problem
// ------------------------------------------------------------
int main()
{
    // Problem dimensions (reversed ordering, dense matrix)
    int users   = 5;    // number of rows in R
    int items   = 4;    // number of columns in R
    int factors = 2;    // latent dimension (must be 2 for our 2×2 solver)
    int iters   = 5;    // ALS cycles
    float lambda = 0.1f; // regularisation strength

    // Create ALS engine and run optimisation
    AlsEngine solver(users, items, factors, iters, lambda);
    solver.run();

    // Display final factor matrices and reconstruction
    solver.showResult();

    return 0;
}
