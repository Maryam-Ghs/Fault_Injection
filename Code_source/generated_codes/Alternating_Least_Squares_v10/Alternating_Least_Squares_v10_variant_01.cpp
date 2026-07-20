/* LLM input variant 1: minimal-boundary */
// Alternating Least Squares – version #10
// ----------------------------------------------------
// This implementation uses only float and int types,
// allocates all large arrays on the heap, generates
// random input data, and prints the resulting factor
// matrices after a few ALS iterations.
// ----------------------------------------------------

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

// ----------------------------------------------------
// Helper: fill a matrix with random floats in [0,1)
// ----------------------------------------------------
void fillRandom(float* mat, int rows, int cols)
{
    int i = 0;
    while (i < rows * cols)
    {
        // generate a random integer then scale to [0,1)
        float rnd = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        mat[i] = rnd;
        ++i;
    }
}

// ----------------------------------------------------
// Helper: allocate a matrix on the heap and zero it
// ----------------------------------------------------
float* allocateZero(int rows, int cols)
{
    int size = rows * cols;
    float* mat = new float[size];
    int idx = 0;
    while (idx < size)
    {
        mat[idx] = 0.0f;
        ++idx;
    }
    return mat;
}

// ----------------------------------------------------
// Helper: compute the transpose‑times‑matrix product
//          (Aᵀ * A) where A is rows×k
// ----------------------------------------------------
void computeAtA(const float* A, int rows, int k, float* AtA)
{
    // initialise AtA to zero
    int i = 0;
    while (i < k * k)
    {
        AtA[i] = 0.0f;
        ++i;
    }

    // accumulate products
    int r = 0;
    while (r < rows)
    {
        int p = 0;
        while (p < k)
        {
            float a_rp = A[r * k + p];
            int q = 0;
            while (q < k)
            {
                AtA[p * k + q] += a_rp * A[r * k + q];
                ++q;
            }
            ++p;
        }
        ++r;
    }
}

// ----------------------------------------------------
// Helper: solve a small k×k linear system (Ax = b)
//          using Gaussian elimination with partial pivoting.
//          The system size (k) is modest (≤10).
// ----------------------------------------------------
void solveLinearSystem(float* A, float* b, int k, float* x)
{
    // make a copy of A and b because we will overwrite them
    float* M = new float[k * k];
    float* rhs = new float[k];
    int i = 0;
    while (i < k * k)
    {
        M[i] = A[i];
        ++i;
    }
    i = 0;
    while (i < k)
    {
        rhs[i] = b[i];
        ++i;
    }

    // ----- forward elimination -----
    int col = 0;
    while (col < k)
    {
        // find pivot row
        int pivot = col;
        float maxAbs = std::fabs(M[col * k + col]);
        int row = col + 1;
        while (row < k)
        {
            float curAbs = std::fabs(M[row * k + col]);
            if (curAbs > maxAbs)
            {
                maxAbs = curAbs;
                pivot = row;
            }
            ++row;
        }

        // swap rows if needed
        if (pivot != col)
        {
            int c = 0;
            while (c < k)
            {
                float tmp = M[col * k + c];
                M[col * k + c] = M[pivot * k + c];
                M[pivot * k + c] = tmp;
                ++c;
            }
            float tmpb = rhs[col];
            rhs[col] = rhs[pivot];
            rhs[pivot] = tmpb;
        }

        // eliminate below
        int r = col + 1;
        while (r < k)
        {
            float factor = M[r * k + col] / M[col * k + col];
            int c = col;
            while (c < k)
            {
                M[r * k + c] -= factor * M[col * k + c];
                ++c;
            }
            rhs[r] -= factor * rhs[col];
            ++r;
        }
        ++col;
    }

    // ----- back substitution -----
    int r = k - 1;
    while (r >= 0)
    {
        float sum = rhs[r];
        int c = r + 1;
        while (c < k)
        {
            sum -= M[r * k + c] * x[c];
            ++c;
        }
        x[r] = sum / M[r * k + r];
        --r;
    }

    delete[] M;
    delete[] rhs;
}

// ----------------------------------------------------
// Helper: update user factors (U) while fixing item factors (V)
// ----------------------------------------------------
void updateUserFactors(const float* R, const float* V, int userCnt,
                       int itemCnt, int rankK, float* U)
{
    // pre‑compute Vᵀ * V once
    float* VtV = allocateZero(rankK, rankK);
    computeAtA(V, itemCnt, rankK, VtV);

    // regularisation term λ * I (λ = 0.1)
    int d = 0;
    while (d < rankK)
    {
        VtV[d * rankK + d] += 0.1f;
        ++d;
    }

    // temporary vectors
    float* rhs = new float[rankK];
    float* sol = new float[rankK];

    // for each user solve (VᵀV + λI) * u_i = Vᵀ * r_i
    int u = 0;
    while (u < userCnt)
    {
        // compute Vᵀ * r_i
        int i = 0;
        while (i < rankK)
        {
            rhs[i] = 0.0f;
            ++i;
        }

        int item = 0;
        while (item < itemCnt)
        {
            float rating = R[u * itemCnt + item];
            if (rating > 0.0f) // treat zero as missing
            {
                int k = 0;
                while (k < rankK)
                {
                    rhs[k] += V[item * rankK + k] * rating;
                    ++k;
                }
            }
            ++item;
        }

        // solve the linear system
        solveLinearSystem(VtV, rhs, rankK, sol);

        // copy solution into U
        int k = 0;
        while (k < rankK)
        {
            U[u * rankK + k] = sol[k];
            ++k;
        }

        ++u;
    }

    delete[] VtV;
    delete[] rhs;
    delete[] sol;
}

// ----------------------------------------------------
// Helper: update item factors (V) while fixing user factors (U)
// ----------------------------------------------------
void updateItemFactors(const float* R, const float* U, int userCnt,
                       int itemCnt, int rankK, float* V)
{
    // pre‑compute Uᵀ * U once
    float* UtU = allocateZero(rankK, rankK);
    computeAtA(U, userCnt, rankK, UtU);

    // regularisation term λ * I (λ = 0.1)
    int d = 0;
    while (d < rankK)
    {
        UtU[d * rankK + d] += 0.1f;
        ++d;
    }

    // temporary vectors
    float* rhs = new float[rankK];
    float* sol = new float[rankK];

    // for each item solve (UᵀU + λI) * v_j = Uᵀ * r_·j
    int it = 0;
    while (it < itemCnt)
    {
        // compute Uᵀ * r_·j
        int i = 0;
        while (i < rankK)
        {
            rhs[i] = 0.0f;
            ++i;
        }

        int user = 0;
        while (user < userCnt)
        {
            float rating = R[user * itemCnt + it];
            if (rating > 0.0f)
            {
                int k = 0;
                while (k < rankK)
                {
                    rhs[k] += U[user * rankK + k] * rating;
                    ++k;
                }
            }
            ++user;
        }

        // solve the linear system
        solveLinearSystem(UtU, rhs, rankK, sol);

        // copy solution into V
        int k = 0;
        while (k < rankK)
        {
            V[it * rankK + k] = sol[k];
            ++k;
        }

        ++it;
    }

    delete[] UtU;
    delete[] rhs;
    delete[] sol;
}

// ----------------------------------------------------
// Helper: compute the Frobenius loss (only on observed entries)
// ----------------------------------------------------
float computeLoss(const float* R, const float* U, const float* V,
                  int userCnt, int itemCnt, int rankK)
{
    float loss = 0.0f;
    int u = 0;
    while (u < userCnt)
    {
        int i = 0;
        while (i < itemCnt)
        {
            float rating = R[u * itemCnt + i];
            if (rating > 0.0f)
            {
                // predict = uᵀ * v
                float pred = 0.0f;
                int k = 0;
                while (k < rankK)
                {
                    pred += U[u * rankK + k] * V[i * rankK + k];
                    ++k;
                }
                float diff = rating - pred;
                loss += diff * diff;
            }
            ++i;
        }
        ++u;
    }
    return loss;
}

// ----------------------------------------------------
// Helper: pretty‑print a matrix (first few rows/cols)
// ----------------------------------------------------
void printMatrix(const char* name, const float* M, int rows, int cols)
{
    std::cout << name << " (" << rows << "x" << cols << "):\n";
    int r = 0;
    while (r < rows && r < 5) // limit to first 5 rows
    {
        int c = 0;
        while (c < cols && c < 5) // limit to first 5 columns
        {
            std::cout << M[r * cols + c] << " ";
            ++c;
        }
        if (cols > 5) std::cout << "...";
        std::cout << "\n";
        ++r;
    }
    if (rows > 5) std::cout << "...\n";
}

// ----------------------------------------------------
// Main driver
// ----------------------------------------------------
int main()
{
    // ------------------------------------------------
    // 1. Seed RNG and define dimensions
    // ------------------------------------------------
    std::srand(static_cast<unsigned>(std::time(0)));
    int userCnt = 1;          // minimal number of users
    int itemCnt = 1;          // minimal number of items
    int rankK   = 1;          // minimal latent dimension
    int iters   = 1;          // minimal ALS iterations

    // ------------------------------------------------
    // 2. Allocate and initialise rating matrix R
    // ------------------------------------------------
    float* R = allocateZero(userCnt, itemCnt);
    fillRandom(R, userCnt, itemCnt);

    // sparsify R: keep only ~30% of entries
    int idx = 0;
    while (idx < userCnt * itemCnt)
    {
        float keep = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        if (keep > 0.3f) R[idx] = 0.0f;
        ++idx;
    }

    // ------------------------------------------------
    // 3. Allocate factor matrices U and V
    // ------------------------------------------------
    float* U = allocateZero(userCnt, rankK);
    float* V = allocateZero(itemCnt, rankK);
    fillRandom(U, userCnt, rankK);
    fillRandom(V, itemCnt, rankK);

    // ------------------------------------------------
    // 4. ALS loop
    // ------------------------------------------------
    int step = 0;
    while (step < iters)
    {
        // a) update U (users) given current V (items)
        updateUserFactors(R, V, userCnt, itemCnt, rankK, U);

        // b) update V (items) given new U (users)
        updateItemFactors(R, U, userCnt, itemCnt, rankK, V);

        // c) compute and display loss
        float curLoss = computeLoss(R, U, V, userCnt, itemCnt, rankK);
        std::cout << "Iteration " << step + 1 << " – loss: " << curLoss << "\n";

        ++step;
    }

    // ------------------------------------------------
    // 5. Print a glimpse of the resulting factors
    // ------------------------------------------------
    printMatrix("U", U, userCnt, rankK);
    printMatrix("V", V, itemCnt, rankK);

    // ------------------------------------------------
    // 6. Clean up
    // ------------------------------------------------
    delete[] R;
    delete[] U;
    delete[] V;

    return 0;
}
