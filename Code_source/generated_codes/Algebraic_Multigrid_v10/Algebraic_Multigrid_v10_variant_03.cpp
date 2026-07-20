/* LLM input variant 3: zeros-and-ones */
/*====================================================================
   Algebraic Multigrid (AMG) – Version #10
   ---------------------------------------------------------------
   - Only float / int are used (no double, long, unsigned, const)
   - All data lives on the heap (new / delete)
   - Verbose, step‑by‑step implementation
   - Small predefined 8×8 identity matrix as test problem
   - No user input (everything is generated inside the program)
   - Results are printed to stdout
====================================================================*/

#include <iostream>
#include <iomanip>

/*--------------------------------------------------------------
   Helper function for printing a vector
--------------------------------------------------------------*/
void showVector(const char* title, const float* vec, int len)
{
    std::cout << title << " : ";
    for (int i = 0; i < len; ++i)
        std::cout << std::fixed << std::setprecision(4) << vec[i] << " ";
    std::cout << "\n";
}

/*--------------------------------------------------------------
   AMG class – encapsulates a two‑level AMG solver
--------------------------------------------------------------*/
class AMG
{
public:
    int nFine;            // number of fine grid unknowns
    float* matFine;       // fine matrix (nFine × nFine)
    float* rhsFine;       // fine right‑hand side
    float* solFine;       // fine solution (unknowns)

    int nCoarse;          // number of coarse grid unknowns
    float* matCoarse;     // coarse matrix (nCoarse × nCoarse)
    float* rhsCoarse;     // coarse right‑hand side
    float* solCoarse;     // coarse solution

    /*----------------------------------------------------------
       Constructor – allocate all required arrays
    ----------------------------------------------------------*/
    AMG(int nf)
    {
        nFine    = nf;
        matFine  = new float[nFine * nFine];
        rhsFine  = new float[nFine];
        solFine  = new float[nFine];

        // a simple coarsening: keep every second point
        nCoarse  = (nFine + 1) / 2;
        matCoarse = new float[nCoarse * nCoarse];
        rhsCoarse = new float[nCoarse];
        solCoarse = new float[nCoarse];
    }

    /*----------------------------------------------------------
       Destructor – free heap memory
    ----------------------------------------------------------*/
    ~AMG()
    {
        delete[] matFine;
        delete[] rhsFine;
        delete[] solFine;

        delete[] matCoarse;
        delete[] rhsCoarse;
        delete[] solCoarse;
    }

    /*----------------------------------------------------------
       Assemble an identity matrix on the fine grid
    ----------------------------------------------------------*/
    void buildFineMatrix()
    {
        int i = 0;
        while (i < nFine)
        {
            int j = 0;
            while (j < nFine)
            {
                // identity: 1 on diagonal, 0 elsewhere
                if (i == j)
                    matFine[i * nFine + j] = 1.0f;
                else
                    matFine[i * nFine + j] = 0.0f;
                ++j;
            }
            ++i;
        }
    }

    /*----------------------------------------------------------
       Create a RHS of all ones: b[i] = 1
    ----------------------------------------------------------*/
    void buildRHS()
    {
        int i = 0;
        while (i < nFine)
        {
            rhsFine[i] = 1.0f;
            ++i;
        }
    }

    /*----------------------------------------------------------
       Perform a few Gauss‑Seidel smoothing steps
    ----------------------------------------------------------*/
    void smooth(int passes)
    {
        int p = 0;
        while (p < passes)
        {
            int i = 0;
            while (i < nFine)
            {
                float sigma = 0.0f;
                int j = 0;
                while (j < nFine)
                {
                    if (j != i)
                        sigma = sigma + (matFine[i * nFine + j] * solFine[j]);
                    ++j;
                }
                // reorder: compute denominator first, then update
                float denom = matFine[i * nFine + i];
                solFine[i] = (rhsFine[i] - sigma) / denom;
                ++i;
            }
            ++p;
        }
    }

    /*----------------------------------------------------------
       Compute residual r = b - A * x  (stored in res[])
    ----------------------------------------------------------*/
    void residual(float* res)
    {
        int i = 0;
        while (i < nFine)
        {
            float Ax_i = 0.0f;
            int j = 0;
            while (j < nFine)
            {
                Ax_i = Ax_i + (matFine[i * nFine + j] * solFine[j]);
                ++j;
            }
            // reorder: b - (A*x) -> -(A*x) + b
            res[i] = -Ax_i + rhsFine[i];
            ++i;
        }
    }

    /*----------------------------------------------------------
       Restrict fine residual to coarse grid (injection)
    ----------------------------------------------------------*/
    void restrictResidual(const float* fineRes)
    {
        int ic = 0;
        while (ic < nCoarse)
        {
            // map coarse index to fine index (2*ic)
            int ifine = ic * 2;
            if (ifine >= nFine) ifine = nFine - 1;
            rhsCoarse[ic] = fineRes[ifine];
            ++ic;
        }
    }

    /*----------------------------------------------------------
       Prolongate coarse correction to fine grid (linear interpolation)
    ----------------------------------------------------------*/
    void prolongate()
    {
        int i = 0;
        while (i < nFine)
        {
            // locate neighboring coarse points
            int ic_left  = i / 2;
            int ic_right = ic_left + 1;
            if (ic_right >= nCoarse) ic_right = ic_left;

            // linear weights based on distance
            float w_right = static_cast<float>(i % 2);
            float w_left  = 1.0f - w_right;

            // reorder: compute weighted sum first
            float correction = w_left * solCoarse[ic_left] + w_right * solCoarse[ic_right];
            solFine[i] = solFine[i] + correction;
            ++i;
        }
    }

    /*----------------------------------------------------------
       Build an identity matrix on the coarse grid
    ----------------------------------------------------------*/
    void buildCoarseMatrix()
    {
        int i = 0;
        while (i < nCoarse)
        {
            int j = 0;
            while (j < nCoarse)
            {
                if (i == j)
                    matCoarse[i * nCoarse + j] = 1.0f;
                else
                    matCoarse[i * nCoarse + j] = 0.0f;
                ++j;
            }
            ++i;
        }
    }

    /*----------------------------------------------------------
       Direct solve on the coarse grid (Gaussian elimination)
    ----------------------------------------------------------*/
    void solveCoarse()
    {
        // copy matrix to a temporary work array (since we cannot use const)
        float* work = new float[nCoarse * nCoarse];
        int i = 0;
        while (i < nCoarse)
        {
            int j = 0;
            while (j < nCoarse)
            {
                work[i * nCoarse + j] = matCoarse[i * nCoarse + j];
                ++j;
            }
            ++i;
        }

        // forward elimination
        int piv = 0;
        while (piv < nCoarse)
        {
            int row = piv + 1;
            while (row < nCoarse)
            {
                float factor = work[row * nCoarse + piv] / work[piv * nCoarse + piv];
                int col = piv;
                while (col < nCoarse)
                {
                    work[row * nCoarse + col] = work[row * nCoarse + col] -
                                                factor * work[piv * nCoarse + col];
                    ++col;
                }
                rhsCoarse[row] = rhsCoarse[row] - factor * rhsCoarse[piv];
                ++row;
            }
            ++piv;
        }

        // back substitution
        int r = nCoarse - 1;
        while (r >= 0)
        {
            float sum = 0.0f;
            int c = r + 1;
            while (c < nCoarse)
            {
                sum = sum + work[r * nCoarse + c] * solCoarse[c];
                ++c;
            }
            solCoarse[r] = (rhsCoarse[r] - sum) / work[r * nCoarse + r];
            --r;
        }

        delete[] work;
    }

    /*----------------------------------------------------------
       One V‑cycle
    ----------------------------------------------------------*/
    void vCycle()
    {
        // 1) pre‑smoothing
        smooth(2);

        // 2) compute residual on fine grid
        float* resFine = new float[nFine];
        residual(resFine);

        // 3) restrict residual to coarse RHS
        restrictResidual(resFine);

        // 4) initialise coarse solution to zero
        int ic = 0;
        while (ic < nCoarse)
        {
            solCoarse[ic] = 0.0f;
            ++ic;
        }

        // 5) build coarse matrix (identity)
        buildCoarseMatrix();

        // 6) solve on coarse grid
        solveCoarse();

        // 7) prolongate correction and update fine solution
        prolongate();

        // 8) post‑smoothing
        smooth(2);

        delete[] resFine;
    }

    /*----------------------------------------------------------
       Run several V‑cycles
    ----------------------------------------------------------*/
    void solve(int cycles)
    {
        // initialise fine solution with zeros
        int i = 0;
        while (i < nFine)
        {
            solFine[i] = 0.0f;
            ++i;
        }

        int c = 0;
        while (c < cycles)
        {
            vCycle();
            ++c;
        }
    }

    /*----------------------------------------------------------
       Print the final fine solution
    ----------------------------------------------------------*/
    void output()
    {
        showVector("Computed solution", solFine, nFine);
    }
};

/*====================================================================
   Main – driver code
====================================================================*/
int main()
{
    // --------------------------------------------------------------
    // Identity matrix on an 8‑node 1‑D grid
    // --------------------------------------------------------------
    AMG solver(8);

    solver.buildFineMatrix();   // assemble A (identity)
    solver.buildRHS();          // set b (all ones)

    // run 5 V‑cycles
    solver.solve(5);

    // display result
    solver.output();

    return 0;
}
