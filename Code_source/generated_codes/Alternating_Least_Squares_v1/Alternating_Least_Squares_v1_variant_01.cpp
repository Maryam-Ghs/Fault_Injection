#include <iostream>
#include <cmath>

/* LLM input variant 1: minimal-boundary */

// ------------------------------------------------------------
// Helper: allocate a matrix on the heap (row‑major layout)
// ------------------------------------------------------------
float* allocMat(int r, int c)
{
    // allocate even if r or c is zero (new returns non‑null for zero size)
    return new float[r * c];
}

// ------------------------------------------------------------
// Helper: free a matrix allocated with allocMat
// ------------------------------------------------------------
void freeMat(float* p)
{
    delete[] p;
}

// ------------------------------------------------------------
// Helper: fill matrix with zeros
// ------------------------------------------------------------
void zeroMat(float* p, int r, int c)
{
    int sz = r * c;
    for (int i = 0; i < sz; ++i)
        p[i] = 0.0f;
}

// ------------------------------------------------------------
// Helper: transpose a matrix (src rows×cols -> dst cols×rows)
// ------------------------------------------------------------
void transMat(const float* src, float* dst, int rows, int cols)
{
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            dst[j * rows + i] = src[i * cols + j];
}

// ------------------------------------------------------------
// Helper: multiply A (aR×aC) with B (bR×bC) -> C (aR×bC)
// ------------------------------------------------------------
void mulMat(const float* A, const float* B, float* C,
            int aR, int aC, int bC)
{
    for (int i = 0; i < aR; ++i)
        for (int j = 0; j < bC; ++j)
        {
            float acc = 0.0f;
            for (int k = 0; k < aC; ++k)
                acc += A[i * aC + k] * B[k * bC + j];
            C[i * bC + j] = acc;
        }
}

// ------------------------------------------------------------
// Helper: solve A·x = b for a symmetric positive matrix A
//         using naïve Gaussian elimination (float only)
// ------------------------------------------------------------
void solveGauss(float* A, const float* b, float* x, int n)
{
    // Build augmented matrix
    float* aug = new float[n * (n + 1)];
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
            aug[i * (n + 1) + j] = A[i * n + j];
        aug[i * (n + 1) + n] = b[i];
    }

    // Forward elimination
    for (int piv = 0; piv < n; ++piv)
    {
        // Pivot scaling (avoid division by zero)
        float pivVal = aug[piv * (n + 1) + piv];
        if (fabs(pivVal) < 1e-6f) pivVal = 1e-6f;
        for (int col = piv; col <= n; ++col)
            aug[piv * (n + 1) + col] /= pivVal;

        // Eliminate rows below
        for (int row = piv + 1; row < n; ++row)
        {
            float factor = aug[row * (n + 1) + piv];
            for (int col = piv; col <= n; ++col)
                aug[row * (n + 1) + col] -= factor * aug[piv * (n + 1) + col];
        }
    }

    // Back substitution
    for (int i = n - 1; i >= 0; --i)
    {
        float sum = aug[i * (n + 1) + n];
        for (int j = i + 1; j < n; ++j)
            sum -= aug[i * (n + 1) + j] * x[j];
        x[i] = sum; // diagonal already 1 after scaling
    }

    delete[] aug;
}

// ------------------------------------------------------------
// Core ALS routine
// ------------------------------------------------------------
void alsFactorize(float* R, int users, int items,
                  int factors, int iters, float lambda,
                  float* U, float* V)
{
    // Pre‑allocate temporaries
    float* Vt = allocMat(factors, items);
    float* VtV = allocMat(factors, factors);
    float* VtR = allocMat(factors, users);
    float* Ut = allocMat(factors, users);
    float* UtU = allocMat(factors, factors);
    float* UtR = allocMat(factors, items);

    // Main ALS loop
    for (int it = 0; it < iters; ++it)
    {
        // ---- Update U (users) ----
        transMat(V, Vt, items, factors);               // Vᵀ
        mulMat(Vt, V, VtV, factors, items, factors);   // Vᵀ·V

        // Add regularization λ·I to VtV (in‑place)
        for (int d = 0; d < factors; ++d)
            VtV[d * factors + d] += lambda;

        // Compute Vᵀ·R (factors×users)
        mulMat(Vt, R, VtR, factors, items, users);

        // Solve for each user i: (VᵀV+λI)·u_i = (VᵀR)_i
        for (int i = 0; i < users; ++i)
        {
            // Build right‑hand side vector (size factors)
            float* rhs = new float[factors];
            for (int f = 0; f < factors; ++f)
                rhs[f] = VtR[f * users + i];

            // Solve linear system
            float* sol = new float[factors];
            solveGauss(VtV, rhs, sol, factors);

            // Write solution into U
            for (int f = 0; f < factors; ++f)
                U[i * factors + f] = sol[f];

            delete[] rhs;
            delete[] sol;
        }

        // ---- Update V (items) ----
        transMat(U, Ut, users, factors);               // Uᵀ
        mulMat(Ut, U, UtU, factors, users, factors);   // Uᵀ·U

        // Add regularization λ·I to UtU
        for (int d = 0; d < factors; ++d)
            UtU[d * factors + d] += lambda;

        // Compute Uᵀ·R (factors×items)
        mulMat(Ut, R, UtR, factors, users, items);

        // Solve for each item j: (UᵀU+λI)·v_j = (UᵀR)_j
        for (int j = 0; j < items; ++j)
        {
            float* rhs = new float[factors];
            for (int f = 0; f < factors; ++f)
                rhs[f] = UtR[f * items + j];

            float* sol = new float[factors];
            solveGauss(UtU, rhs, sol, factors);

            for (int f = 0; f < factors; ++f)
                V[j * factors + f] = sol[f];

            delete[] rhs;
            delete[] sol;
        }
    }

    // Clean up temporaries
    freeMat(Vt);
    freeMat(VtV);
    freeMat(VtR);
    freeMat(Ut);
    freeMat(UtU);
    freeMat(UtR);
}

// ------------------------------------------------------------
// Generate a deliberately edge‑case heavy rating matrix
// ------------------------------------------------------------
void generateEdgeCaseR(float* R, int users, int items)
{
    // Fill with zeros (missing ratings)
    zeroMat(R, users, items);

    // Insert a few explicit ratings
    if (users > 0 && items > 0)
    {
        // Populate a sparse pattern
        for (int i = 0; i < users; ++i)
        {
            for (int j = 0; j < items; ++j)
            {
                // Only a few entries get non‑zero values
                if ((i + j) % (users + items) == 0)
                {
                    // Use extreme values to stress the algorithm
                    if ((i + j) % 2 == 0)
                        R[i * items + j] = 1000.0f;   // very large rating
                    else
                        R[i * items + j] = -999.0f;   // very small (negative) rating
                }
            }
        }
    }
}

// ------------------------------------------------------------
// Print a matrix (row‑major)
// ------------------------------------------------------------
void printMat(const char* name, const float* M, int rows, int cols)
{
    std::cout << name << " (" << rows << "×" << cols << "):\n";
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
            std::cout << M[i * cols + j] << ' ';
        std::cout << '\n';
    }
    std::cout << std::flush;
}

// ------------------------------------------------------------
// Entry point
// ------------------------------------------------------------
int main()
{
    // Minimal‑boundary dimensions
    int nUsers = 1;
    int nItems = 1;
    int k      = 1;      // latent factors
    int steps  = 1;      // ALS iterations
    float reg  = 0.1f;   // regularization λ

    // Allocate rating matrix R (users×items)
    float* R = allocMat(nUsers, nItems);
    generateEdgeCaseR(R, nUsers, nItems);
    printMat("Rating matrix R", R, nUsers, nItems);

    // Allocate factor matrices
    float* U = allocMat(nUsers, k);
    float* V = allocMat(nItems, k);
    zeroMat(U, nUsers, k);
    zeroMat(V, nItems, k);

    // Run ALS
    alsFactorize(R, nUsers, nItems, k, steps, reg, U, V);

    // Output results
    printMat("User factors U", U, nUsers, k);
    printMat("Item factors V", V, nItems, k);

    // Clean up
    freeMat(R);
    freeMat(U);
    freeMat(V);

    return 0;
}
