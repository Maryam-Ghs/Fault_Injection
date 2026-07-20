/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <vector>
#include <functional>
#include <iomanip>

int main()
{
    // -----------------------------------------------------------------
    // Verbose implementation of a very simple two‑level Algebraic Multigrid
    // (AMG) solver for the 1‑D Poisson problem.
    // All variables are either int or float (no double, long, unsigned,
    // const).  Everything is defined inside main().
    // -----------------------------------------------------------------

    // -----------------------------------------------------------------
    // Helper lambda: print a vector with a label
    // -----------------------------------------------------------------
    auto show_vector = [&](const std::vector<float>& vec, const char* label)
    {
        std::cout << label << " = [ ";
        for (int i = 0; i < (int)vec.size(); ++i)
        {
            std::cout << std::fixed << std::setprecision(6) << vec[i];
            if (i + 1 < (int)vec.size()) std::cout << ", ";
        }
        std::cout << " ]\n";
    };

    // -----------------------------------------------------------------
    // Helper lambda: multiply matrix A (stored row‑wise) by vector v
    // -----------------------------------------------------------------
    auto multiply = [&](const std::vector<std::vector<float>>& A,
                        const std::vector<float>& v,
                        std::vector<float>& out)
    {
        int n = (int)A.size();
        for (int i = 0; i < n; ++i)
        {
            float sum = 0.0f;
            for (int j = 0; j < n; ++j)
            {
                sum = sum + A[i][j] * v[j];
            }
            out[i] = sum;
        }
    };

    // -----------------------------------------------------------------
    // Helper lambda: compute residual r = b - A*x
    // -----------------------------------------------------------------
    auto residual = [&](const std::vector<std::vector<float>>& A,
                        const std::vector<float>& x,
                        const std::vector<float>& b,
                        std::vector<float>& r)
    {
        int n = (int)b.size();
        std::vector<float> Ax(n, 0.0f);
        multiply(A, x, Ax);
        for (int i = 0; i < n; ++i)
        {
            r[i] = b[i] - Ax[i];
        }
    };

    // -----------------------------------------------------------------
    // Helper lambda: weighted Jacobi smoother
    // -----------------------------------------------------------------
    auto jacobi_smoother = [&](const std::vector<std::vector<float>>& A,
                               std::vector<float>& x,
                               const std::vector<float>& b,
                               int sweeps,
                               float omega)
    {
        int n = (int)A.size();
        std::vector<float> x_new(n, 0.0f);
        for (int s = 0; s < sweeps; ++s)
        {
            for (int i = 0; i < n; ++i)
            {
                float sigma = 0.0f;
                for (int j = 0; j < n; ++j)
                {
                    if (j != i)
                    {
                        sigma = sigma + A[i][j] * x[j];
                    }
                }
                // D = A[i][i]
                float D = A[i][i];
                // x_new[i] = (1-omega)*x[i] + (omega/D)*(b[i] - sigma)
                float term1 = (1.0f - omega) * x[i];
                float term2 = (omega / D) * (b[i] - sigma);
                x_new[i] = term1 + term2;
            }
            // copy back
            for (int i = 0; i < n; ++i) x[i] = x_new[i];
        }
    };

    // -----------------------------------------------------------------
    // Helper lambda: restriction (full‑weighting) from fine to coarse
    // -----------------------------------------------------------------
    auto restrict_op = [&](const std::vector<float>& fine,
                           std::vector<float>& coarse)
    {
        int nf = (int)fine.size();
        int nc = (int)coarse.size();
        for (int i = 0; i < nc; ++i)
        {
            int f_idx = 2 * i;
            float left  = (f_idx - 1 >= 0)      ? fine[f_idx - 1] : 0.0f;
            float mid   = fine[f_idx];
            float right = (f_idx + 1 < nf)     ? fine[f_idx + 1] : 0.0f;
            // full weighting: (left + 2*mid + right) / 4
            coarse[i] = (left + 2.0f * mid + right) * 0.25f;
        }
    };

    // -----------------------------------------------------------------
    // Helper lambda: prolongation (linear interpolation) from coarse to fine
    // -----------------------------------------------------------------
    auto prolong_op = [&](const std::vector<float>& coarse,
                          std::vector<float>& fine)
    {
        int nc = (int)coarse.size();
        int nf = (int)fine.size();
        for (int i = 0; i < nf; ++i)
        {
            if (i % 2 == 0)               // coincides with coarse node
            {
                fine[i] = coarse[i / 2];
            }
            else                          // midpoint, average neighbours
            {
                int left  = i / 2;
                int right = left + 1;
                float v_left  = coarse[left];
                float v_right = (right < nc) ? coarse[right] : v_left;
                fine[i] = 0.5f * (v_left + v_right);
            }
        }
    };

    // -----------------------------------------------------------------
    // Helper lambda: direct solve on the coarse grid (Gaussian elimination)
    // -----------------------------------------------------------------
    auto coarse_solve = [&](std::vector<std::vector<float>>& Ac,
                            std::vector<float>& bc,
                            std::vector<float>& xc)
    {
        int n = (int)Ac.size();
        // Forward elimination
        for (int k = 0; k < n - 1; ++k)
        {
            float pivot = Ac[k][k];
            for (int i = k + 1; i < n; ++i)
            {
                float factor = Ac[i][k] / pivot;
                for (int j = k; j < n; ++j)
                {
                    Ac[i][j] = Ac[i][j] - factor * Ac[k][j];
                }
                bc[i] = bc[i] - factor * bc[k];
            }
        }
        // Back substitution
        for (int i = n - 1; i >= 0; --i)
        {
            float sum = 0.0f;
            for (int j = i + 1; j < n; ++j)
            {
                sum = sum + Ac[i][j] * xc[j];
            }
            xc[i] = (bc[i] - sum) / Ac[i][i];
        }
    };

    // -----------------------------------------------------------------
    // Test on two edge‑case sizes: reverse‑ordered (large then small)
    // -----------------------------------------------------------------
    std::vector<int> test_sizes;
    test_sizes.push_back(256);   // larger grid first (adversarial ordering)
    test_sizes.push_back(2);     // minimal non‑trivial grid second

    for (int t = 0; t < (int)test_sizes.size(); ++t)
    {
        int fine_n = test_sizes[t];
        std::cout << "\n=== Solving 1‑D Poisson with N = " << fine_n << " ===\n";

        // -------------------------------------------------------------
        // Build fine‑grid matrix A (tridiagonal Laplacian) using reverse loops
        // -------------------------------------------------------------
        std::vector<std::vector<float>> Af(fine_n, std::vector<float>(fine_n, 0.0f));
        for (int i = fine_n - 1; i >= 0; --i)
        {
            for (int j = fine_n - 1; j >= 0; --j)
            {
                if (i == j)               Af[i][j] = 2.0f;
                else if (j == i - 1)      Af[i][j] = -1.0f;
                else if (j == i + 1)      Af[i][j] = -1.0f;
                else                      Af[i][j] = 0.0f;
            }
        }

        // -------------------------------------------------------------
        // Right‑hand side b = descending values (adversarial pattern)
        // -------------------------------------------------------------
        std::vector<float> bf(fine_n, 0.0f);
        for (int i = 0; i < fine_n; ++i)
        {
            bf[i] = static_cast<float>(fine_n - i);
        }

        // -------------------------------------------------------------
        // Initial guess x = 0
        // -------------------------------------------------------------
        std::vector<float> xf(fine_n, 0.0f);

        // -------------------------------------------------------------
        // Parameters for the V‑cycle
        // -------------------------------------------------------------
        int pre_smooth  = 3;
        int post_smooth = 3;
        float omega = 0.8f;   // relaxation weight

        // -------------------------------------------------------------
        // 1) Pre‑smoothing
        // -------------------------------------------------------------
        jacobi_smoother(Af, xf, bf, pre_smooth, omega);

        // -------------------------------------------------------------
        // 2) Compute residual r = b - A*x
        // -------------------------------------------------------------
        std::vector<float> rf(fine_n, 0.0f);
        residual(Af, xf, bf, rf);

        // -------------------------------------------------------------
        // 3) Restrict residual to coarse grid
        // -------------------------------------------------------------
        int coarse_n = (fine_n + 1) / 2;   // ceil(fine_n/2)
        std::vector<float> rc(coarse_n, 0.0f);
        restrict_op(rf, rc);

        // -------------------------------------------------------------
        // 4) Build coarse‑grid matrix Ac (Galerkin: Ac = R * A * P)
        //    For this toy problem we simply create the same Laplacian
        //    on the coarse grid (valid for uniform 1‑D).
        // -------------------------------------------------------------
        std::vector<std::vector<float>> Ac(coarse_n, std::vector<float>(coarse_n, 0.0f));
        for (int i = coarse_n - 1; i >= 0; --i)
        {
            for (int j = coarse_n - 1; j >= 0; --j)
            {
                if (i == j)               Ac[i][j] = 2.0f;
                else if (j == i - 1)      Ac[i][j] = -1.0f;
                else if (j == i + 1)      Ac[i][j] = -1.0f;
                else                      Ac[i][j] = 0.0f;
            }
        }

        // -------------------------------------------------------------
        // 5) Solve coarse system Ac * ec = rc
        // -------------------------------------------------------------
        std::vector<float> ec(coarse_n, 0.0f);
        // make copies because Gaussian elimination overwrites
        std::vector<std::vector<float>> Ac_copy = Ac;
        std::vector<float> rc_copy = rc;
        coarse_solve(Ac_copy, rc_copy, ec);

        // -------------------------------------------------------------
        // 6) Prolongate error to fine grid and correct solution
        // -------------------------------------------------------------
        std::vector<float> ef(fine_n, 0.0f);
        prolong_op(ec, ef);
        for (int i = 0; i < fine_n; ++i)
        {
            xf[i] = xf[i] + ef[i];
        }

        // -------------------------------------------------------------
        // 7) Post‑smoothing
        // -------------------------------------------------------------
        jacobi_smoother(Af, xf, bf, post_smooth, omega);

        // -------------------------------------------------------------
        // Print final solution
        // -------------------------------------------------------------
        show_vector(xf, "Solution x");
    }

    return 0;
}
