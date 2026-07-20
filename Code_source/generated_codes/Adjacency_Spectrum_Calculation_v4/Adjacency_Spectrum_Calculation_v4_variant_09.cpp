#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

/* LLM input variant 9: medium-deterministic-random */

// -------------------------------------------------------------------
// Helper: create a deterministic pseudo‑random‑looking graph
// -------------------------------------------------------------------
void build_graph(std::vector<std::vector<int>>& adj) {
    int sz = 9;                     // number of vertices (expanded from 7)
    adj.assign(sz, std::vector<int>(sz, 0));

    // Deterministic edge weights using a simple linear congruential pattern
    for (int i = 0; i < sz; ++i) {
        for (int j = i + 1; j < sz; ++j) {
            int w = ((i * 7 + j * 13) % 4); // yields 0,1,2,3
            if (w > 0) {
                adj[i][j] = adj[j][i] = w;
            }
        }
    }

    // Add self‑loops on even‑indexed vertices to preserve edge‑case handling
    for (int i = 0; i < sz; ++i) {
        if (i % 2 == 0) {
            adj[i][i] = 1;
        }
    }

    // Force an isolated vertex (vertex 8) by clearing its row/column after
    // the deterministic fill; ensures at least one isolate remains.
    for (int i = 0; i < sz; ++i) {
        adj[8][i] = adj[i][8] = 0;
    }
}

// -------------------------------------------------------------------
// Multiply integer matrix by float vector (result = mat * vec)
// -------------------------------------------------------------------
void mul_mat_vec(const std::vector<std::vector<int>>& mat,
                 const std::vector<float>& vec,
                 std::vector<float>& out) {
    int n = (int)mat.size();
    for (int i = 0; i < n; ++i) {
        float acc = 0.0f;
        for (int j = 0; j < n; ++j) {
            acc += (float)mat[i][j] * vec[j];
        }
        out[i] = acc;
    }
}

// -------------------------------------------------------------------
// Normalize a vector (in‑place) and return its 2‑norm
// -------------------------------------------------------------------
float norm_and_scale(std::vector<float>& v) {
    float sum = 0.0f;
    int len = (int)v.size();
    for (int i = 0; i < len; ++i) {
        sum += v[i] * v[i];
    }
    float nrm = std::sqrt(sum);
    if (nrm > 0.0f) {
        for (int i = 0; i < len; ++i) {
            v[i] /= nrm;
        }
    }
    return nrm;
}

// -------------------------------------------------------------------
// Power iteration for the dominant eigenpair of an integer matrix
// -------------------------------------------------------------------
void dominant_pair(const std::vector<std::vector<int>>& mat,
                   int iters,
                   float tol,
                   std::vector<float>& eig_vec,
                   float& eig_val) {
    int n = (int)mat.size();
    eig_vec.assign(n, 0.0f);
    for (int i = 0; i < n; ++i) eig_vec[i] = 1.0f;   // start with all‑ones

    std::vector<float> tmp(n, 0.0f);
    float prev = 0.0f;

    for (int it = 0; it < iters; ++it) {
        mul_mat_vec(mat, eig_vec, tmp);
        eig_val = norm_and_scale(tmp);
        // swap vectors without copying
        std::swap(eig_vec, tmp);
        if (std::fabs(eig_val - prev) < tol) break;
        prev = eig_val;
    }
}

// -------------------------------------------------------------------
// Deflate the matrix by removing the contribution of an eigenpair
// -------------------------------------------------------------------
void deflate(std::vector<std::vector<float>>& fmat,
             const std::vector<float>& vec,
             float val) {
    int n = (int)fmat.size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            fmat[i][j] -= val * vec[i] * vec[j];
        }
    }
}

// -------------------------------------------------------------------
// Compute all eigenvalues by repeated power iteration + deflation
// -------------------------------------------------------------------
void full_spectrum(const std::vector<std::vector<int>>& int_mat,
                   int max_it,
                   float eps,
                   std::vector<float>& spec) {
    int n = (int)int_mat.size();

    // copy integer matrix to float matrix for deflation
    std::vector<std::vector<float>> work(n, std::vector<float>(n, 0.0f));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            work[i][j] = (float)int_mat[i][j];

    spec.clear();
    for (int k = 0; k < n; ++k) {
        // temporary integer view of the current float matrix (rounded)
        std::vector<std::vector<int>> cur_int(n, std::vector<int>(n, 0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                cur_int[i][j] = (int)std::round(work[i][j]);

        std::vector<float> ev;
        float ev_val;
        dominant_pair(cur_int, max_it, eps, ev, ev_val);
        spec.push_back(ev_val);

        // stop early if eigenvalue is effectively zero
        if (std::fabs(ev_val) < eps) break;

        deflate(work, ev, ev_val);
    }
}

// -------------------------------------------------------------------
// Main driver
// -------------------------------------------------------------------
int main() {
    std::vector<std::vector<int>> adjacency;
    build_graph(adjacency);

    std::vector<float> eigen_list;
    full_spectrum(adjacency, 2000, 1e-4f, eigen_list);

    std::cout << "Adjacency spectrum (approximate, float precision):\n";
    for (int i = 0; i < (int)eigen_list.size(); ++i) {
        std::cout << "  lambda[" << i << "] = " << eigen_list[i] << "\n";
    }
    return 0;
}
