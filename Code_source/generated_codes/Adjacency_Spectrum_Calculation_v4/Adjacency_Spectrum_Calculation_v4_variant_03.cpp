/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

// -------------------------------------------------------------------
// Helper: create a graph that stresses edge cases (self‑loops, isolates)
// -------------------------------------------------------------------
void build_graph(std::vector<std::vector<int>>& adj) {
    int sz = 5;                     // number of vertices (reduced, mostly zeros)
    adj.assign(sz, std::vector<int>(sz, 0));

    // Identity component: self‑loops on all vertices (ones on diagonal)
    for (int i = 0; i < sz; ++i) {
        adj[i][i] = 1;
    }

    // Single off‑diagonal edge to introduce a non‑trivial connection
    adj[0][1] = adj[1][0] = 1;

    // All other entries remain zero (isolated-like)
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
