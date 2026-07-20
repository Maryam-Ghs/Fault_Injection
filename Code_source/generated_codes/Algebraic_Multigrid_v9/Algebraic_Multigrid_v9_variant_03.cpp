/******************************************************************
 * Algebraic Multigrid (AMG) – version #9
 * --------------------------------------------------------------
 *  * Only float / int types are used (no double, long, unsigned,
 *    const etc.).
 *  * All data lives on the stack (fixed size arrays).
 *  * No user input – the linear system is generated internally.
 *  * Verbose step‑by‑step helper functions are provided.
 *  * Arithmetic operations are deliberately reordered where
 *    mathematically permissible.
 *  * Loop structures are varied (for, while, reverse loops).
 ******************************************************************/

/* LLM input variant 3: zeros-and-ones */

#include <cstdio>
#include <cstdlib>
#include <ctime>

/*---------------------------------------------------------------*/
/* Problem size (fine grid)                                      */
#define N_FINE 5          /* number of fine points               */
#define N_COAR 3          /* number of coarse points (N_FINE/2+1)*/

/*---------------------------------------------------------------*/
/* Helper: fill a matrix with a deterministic, zeros‑and‑ones system */
void buildSystem(int n, float mat[][N_FINE], float rhs[]) {
    int i, j;
    for (i = 0; i < n; ++i) {
        float row_sum = 0.0f;
        for (j = 0; j < n; ++j) {
            if (i == j) {
                mat[i][j] = 0.0f;               /* placeholder */
            } else {
                /* deterministic pattern: 1 on even (i+j), 0 otherwise */
                float val = ((i + j) % 2 == 0) ? 1.0f : 0.0f;
                mat[i][j] = val;
                row_sum = row_sum + (val < 0.0f ? -val : val); /* |val| */
            }
        }
        /* enforce strict diagonal dominance */
        mat[i][i] = row_sum + 1.0f;
    }

    /* deterministic RHS: alternate 0 and 1 */
    for (i = 0; i < n; ++i) {
        rhs[i] = (i % 2 == 0) ? 0.0f : 1.0f;
    }
}

/*---------------------------------------------------------------*/
/* Compute residual:  r = rhs - A * sol                         */
void getResidual(int n, float mat[][N_FINE], float sol[], float rhs[], float resid[]) {
    int i = 0;
    while (i < n) {
        float dot = 0.0f;
        int j = 0;
        while (j < n) {
            /* reorder multiplication/addition */
            dot = dot + (mat[i][j] * sol[j]);
            ++j;
        }
        /* rhs - (A*sol) */
        resid[i] = rhs[i] - dot;
        ++i;
    }
}

/*---------------------------------------------------------------*/
/* Simple Gauss–Seidel smoother (in‑place)                       */
void smoothGS(int n, float mat[][N_FINE], float rhs[], float sol[], int its) {
    int it = 0;
    while (it < its) {
        int i = 0;
        while (i < n) {
            float sigma = 0.0f;
            int j = 0;
            while (j < n) {
                if (j != i) {
                    sigma = sigma + (mat[i][j] * sol[j]);
                }
                ++j;
            }
            /* new value: (rhs - sigma) / diag */
            sol[i] = (rhs[i] - sigma) / mat[i][i];
            ++i;
        }
        ++it;
    }
}

/*---------------------------------------------------------------*/
/* Restriction (injection) – fine to coarse                     */
void restrictFineToCoarse(int nf, int nc, float fineRes[], float coarseRes[]) {
    int ic = 0;
    while (ic < nc) {
        /* injection: take every second fine point */
        int ifine = ic * 2;
        if (ifine >= nf) ifine = nf - 1;   /* safety for last point */
        coarseRes[ic] = fineRes[ifine];
        ++ic;
    }
}

/*---------------------------------------------------------------*/
/* Prolongation – linear interpolation (coarse to fine)         */
void prolongateCoarseToFine(int nc, int nf, float coarseCorr[], float fineCorr[]) {
    int i = 0;
    while (i < nf) {
        if (i % 2 == 0) {
            /* coincide with coarse node */
            fineCorr[i] = coarseCorr[i / 2];
        } else {
            /* average of neighbours */
            int left  = (i - 1) / 2;
            int right = (i + 1) / 2;
            if (right >= nc) right = nc - 1;
            fineCorr[i] = 0.5f * (coarseCorr[left] + coarseCorr[right]);
        }
        ++i;
    }
}

/*---------------------------------------------------------------*/
/* Direct solve on the coarse grid (Gaussian elimination)      */
void solveCoarse(int nc, float coarseMat[][N_COAR], float rhsCoarse[], float corrCoarse[]) {
    /* copy to temporary storage (still on stack) */
    float tmpMat[N_COAR][N_COAR];
    float tmpRhs[N_COAR];
    int i, j, k;

    /* forward elimination */
    for (i = 0; i < nc; ++i) {
        for (j = 0; j < nc; ++j) {
            tmpMat[i][j] = coarseMat[i][j];
        }
        tmpRhs[i] = rhsCoarse[i];
    }

    for (k = 0; k < nc - 1; ++k) {
        /* pivot is assumed non‑zero (matrix is SPD) */
        int irow = k + 1;
        while (irow < nc) {
            float factor = tmpMat[irow][k] / tmpMat[k][k];
            int icol = k;
            while (icol < nc) {
                /* reorder subtraction */
                tmpMat[irow][icol] = tmpMat[irow][icol] - factor * tmpMat[k][icol];
                ++icol;
            }
            tmpRhs[irow] = tmpRhs[irow] - factor * tmpRhs[k];
            ++irow;
        }
    }

    /* back substitution */
    for (i = nc - 1; i >= 0; --i) {
        float sum = 0.0f;
        int jcol = i + 1;
        while (jcol < nc) {
            sum = sum + (tmpMat[i][jcol] * corrCoarse[jcol]);
            ++jcol;
        }
        corrCoarse[i] = (tmpRhs[i] - sum) / tmpMat[i][i];
    }
}

/*---------------------------------------------------------------*/
/* Build a simple Galerkin coarse matrix: Ac = R * A * P        */
void buildCoarseMatrix(int nf, int nc, float fineMat[][N_FINE],
                       float coarseMat[][N_COAR]) {
    int i = 0;
    while (i < nc) {
        int j = 0;
        while (j < nc) {
            /* R = injection, P = linear interpolation.
               Compute entry Ac(i,j) = sum_{p,q} R(i,p) * A(p,q) * P(q,j) */
            float entry = 0.0f;
            int p = 0;
            while (p < nf) {
                /* R(i,p) is 1 if p==2*i else 0 */
                float r_ip = (p == 2 * i) ? 1.0f : 0.0f;
                int q = 0;
                while (q < nf) {
                    /* P(q,j) – linear interpolation weights */
                    float p_qj = 0.0f;
                    if (q == 2 * j) {
                        p_qj = 1.0f;
                    } else if (q == 2 * j + 1) {
                        p_qj = 0.5f;
                    } else if (q == 2 * j - 1) {
                        p_qj = 0.5f;
                    }
                    entry = entry + (r_ip * fineMat[p][q] * p_qj);
                    ++q;
                }
                ++p;
            }
            coarseMat[i][j] = entry;
            ++j;
        }
        ++i;
    }
}

/*---------------------------------------------------------------*/
/* One V‑cycle of the two‑level AMG                              */
void Vcycle(int nf, int nc,
            float fineMat[][N_FINE], float rhsFine[], float solFine[]) {
    float residFine[N_FINE];
    float corrFine[N_FINE];
    float residCoarse[N_COAR];
    float corrCoarse[N_COAR];
    float coarseMat[N_COAR][N_COAR];

    /* 1) pre‑smoothing */
    smoothGS(nf, fineMat, rhsFine, solFine, 2);

    /* 2) compute residual */
    getResidual(nf, fineMat, solFine, rhsFine, residFine);

    /* 3) restrict residual */
    restrictFineToCoarse(nf, nc, residFine, residCoarse);

    /* 4) build coarse matrix */
    buildCoarseMatrix(nf, nc, fineMat, coarseMat);

    /* 5) solve coarse problem */
    solveCoarse(nc, coarseMat, residCoarse, corrCoarse);

    /* 6) prolongate correction */
    prolongateCoarseToFine(nc, nf, corrCoarse, corrFine);

    /* 7) update fine solution */
    int i = 0;
    while (i < nf) {
        /* reorder addition */
        solFine[i] = solFine[i] + corrFine[i];
        ++i;
    }

    /* 8) post‑smoothing */
    smoothGS(nf, fineMat, rhsFine, solFine, 2);
}

/*---------------------------------------------------------------*/
/* Main driver – generate system, apply AMG, print result       */
int main() {
    /* -----------------------------------------------------------------
       1) Build a deterministic, zeros‑and‑ones fine‑grid system
       ----------------------------------------------------------------- */
    float fineMat[N_FINE][N_FINE];
    float rhsVec[N_FINE];
    float solVec[N_FINE] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    buildSystem(N_FINE, fineMat, rhsVec);

    /* -----------------------------------------------------------------
       2) Perform a few V‑cycles
       ----------------------------------------------------------------- */
    int cycle = 0;
    while (cycle < 5) {
        Vcycle(N_FINE, N_COAR, fineMat, rhsVec, solVec);
        ++cycle;
    }

    /* -----------------------------------------------------------------
       3) Output the computed solution
       ----------------------------------------------------------------- */
    printf("Computed solution after %d V‑cycles (AMG version #9):\n", cycle);
    int i = 0;
    while (i < N_FINE) {
        printf("  x[%d] = %f\n", i, solVec[i]);
        ++i;
    }

    return 0;
}
