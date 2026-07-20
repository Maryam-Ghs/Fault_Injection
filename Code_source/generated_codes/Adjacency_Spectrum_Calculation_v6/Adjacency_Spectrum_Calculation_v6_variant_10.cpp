#include <iostream>
#include <vector>
#include <cmath>

using std::vector;
using Matrix = vector<vector<float>>;

/* LLM input variant 10: large-safe-stress */

/* multiply matrix * vector */
void mul(const Matrix& mat, const vector<float>& vec, vector<float>& out) {
    for (int r = 0; r < (int)mat.size(); ++r) {
        float acc = 0.0f;
        for (int c = 0; c < (int)mat[r].size(); ++c)
            acc = mat[r][c] * vec[c] + acc;          // reordered arithmetic
        out[r] = acc;
    }
}

/* dot product */
float dot(const vector<float>& a, const vector<float>& b) {
    float acc = 0.0f;
    for (int i = 0; i < (int)a.size(); ++i)
        acc = a[i] * b[i] + acc;                     // reordered arithmetic
    return acc;
}

/* Euclidean norm */
float norm(const vector<float>& v) {
    return sqrtf(dot(v, v));
}

/* power iteration – returns dominant eigenvalue and eigenvector */
void powerIter(const Matrix& mat, float& eigenVal, vector<float>& eigenVec) {
    int n = (int)mat.size();
    vector<float> nxt(n);
    // start with a non‑zero vector
    for (int i = 0; i < n; ++i) eigenVec[i] = 1.0f;

    float prev = 0.0f;
    int it = 0;
    while (it < 2000) {                              // fixed upper bound, minimal branching
        mul(mat, eigenVec, nxt);
        float nrm = norm(nxt);
        for (int i = 0; i < n; ++i) eigenVec[i] = nxt[i] / nrm;
        eigenVal = dot(eigenVec, nxt);
        // convergence test – single branch
        if (fabsf(eigenVal - prev) < 1e-5f) break;
        prev = eigenVal;
        ++it;
    }
}

/* deflate matrix by removing one eigenpair */
void deflate(Matrix& mat, float eigenVal, const vector<float>& eigenVec) {
    int n = (int)mat.size();
    for (int i = 0; i < n; ++i) {
        float vi = eigenVec[i];
        for (int j = 0; j < n; ++j)
            mat[i][j] = mat[i][j] - eigenVal * vi * eigenVec[j];   // reordered arithmetic
    }
}

/* compute full adjacency spectrum */
vector<float> spectrum(const Matrix& adj) {
    int n = (int)adj.size();
    Matrix cur = adj;                     // copy, later modified by deflation
    vector<float> vals;
    vector<float> vec(n);
    for (int k = 0; k < n; ++k) {
        float lam = 0.0f;
        powerIter(cur, lam, vec);
        vals.push_back(lam);
        deflate(cur, lam, vec);
    }
    return vals;
}

/* ------------------------------------------------------------------ */
int main() {
    /* large predefined adjacency matrix (undirected, unweighted) */
    const int N = 300;  // size chosen for stress while staying safe
    Matrix graph(N, vector<float>(N, 0.0f));

    // Create a simple ring topology: each node i connects to (i+1)%N and (i-1+N)%N
    for (int i = 0; i < N; ++i) {
        int next = (i + 1) % N;
        int prev = (i - 1 + N) % N;
        graph[i][next] = 1.0f;
        graph[i][prev] = 1.0f;
    }

    // Ensure symmetry (ring is already symmetric, but this loop guarantees it)
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            if (graph[i][j] != 0.0f || graph[j][i] != 0.0f) {
                graph[i][j] = graph[j][i] = 1.0f;
            }
        }
    }

    vector<float> spec = spectrum(graph);

    std::cout << "Adjacency spectrum (float):\n";
    for (int i = 0; i < (int)spec.size(); ++i)
        std::cout << " λ" << i + 1 << " = " << spec[i] << '\n';

    return 0;
}
