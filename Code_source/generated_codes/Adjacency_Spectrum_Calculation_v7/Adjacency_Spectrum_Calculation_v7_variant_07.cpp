#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 7: reverse-adversarial */

/* ---------- helper: matrix multiplication (float) ---------- */
vector<vector<float>> mul(const vector<vector<float>>& x,
                          const vector<vector<float>>& y)
{
    int n = (int)x.size();
    vector<vector<float>> res(n, vector<float>(n, 0.0f));
    int i = 0;
    while (i < n) {
        int j = 0;
        while (j < n) {
            float sum = 0.0f;
            int k = 0;
            while (k < n) {
                sum += x[i][k] * y[k][j];
                ++k;
            }
            res[i][j] = sum;
            ++j;
        }
        ++i;
    }
    return res;
}

/* ---------- helper: matrix transpose ---------- */
vector<vector<float>> tr(const vector<vector<float>>& m)
{
    int n = (int)m.size();
    vector<vector<float>> t(n, vector<float>(n, 0.0f));
    int r = 0;
    while (r < n) {
        int c = 0;
        while (c < n) {
            t[c][r] = m[r][c];
            ++c;
        }
        ++r;
    }
    return t;
}

/* ---------- helper: QR decomposition via Gram‑Schmidt ---------- */
pair<vector<vector<float>>, vector<vector<float>>> qr(const vector<vector<float>>& a)
{
    int n = (int)a.size();
    vector<vector<float>> q(n, vector<float>(n, 0.0f));
    vector<vector<float>> r(n, vector<float>(n, 0.0f));

    int col = 0;
    while (col < n) {
        /* v = a[:,col] */
        vector<float> v(n);
        int i = 0;
        while (i < n) {
            v[i] = a[i][col];
            ++i;
        }

        /* subtract projections */
        int k = 0;
        while (k < col) {
            float dot = 0.0f;
            int j = 0;
            while (j < n) {
                dot += q[j][k] * a[j][col];
                ++j;
            }
            r[k][col] = dot;
            int m = 0;
            while (m < n) {
                v[m] -= dot * q[m][k];
                ++m;
            }
            ++k;
        }

        /* norm */
        float norm = 0.0f;
        int p = 0;
        while (p < n) {
            norm += v[p] * v[p];
            ++p;
        }
        norm = sqrtf(norm);
        r[col][col] = norm;

        /* q column */
        int qrow = 0;
        while (qrow < n) {
            q[qrow][col] = v[qrow] / norm;
            ++qrow;
        }
        ++col;
    }
    return {q, r};
}

/* ---------- core: QR iteration to approximate eigenvalues ---------- */
vector<float> spectrum(const vector<vector<float>>& adj)
{
    int n = (int)adj.size();
    vector<vector<float>> a = adj;            // working copy
    int iter = 0;
    while (iter < 200) {                      // fixed iteration count
        auto [q, r] = qr(a);
        a = mul(r, q);                         // A_{k+1} = R·Q
        ++iter;
    }

    vector<float> ev(n);
    int i = 0;
    while (i < n) {
        ev[i] = a[i][i];                       // diagonal holds eigen‑values
        ++i;
    }
    return ev;
}

/* ---------- utility: pretty print a vector ---------- */
void out(const vector<float>& v)
{
    int i = 0;
    while (i < (int)v.size()) {
        cout << fixed << setprecision(4) << v[i];
        if (i + 1 < (int)v.size()) cout << ", ";
        ++i;
    }
    cout << '\n';
}

/* ---------- main: generate a reverse‑ordered adversarial graph and compute its spectrum ---------- */
int main()
{
    /* construct an 8‑vertex graph with reverse‑pair edges and a path, creating a worst‑case‑like adjacency */
    const int n = 8;
    vector<vector<float>> g(n, vector<float>(n, 0.0f));

    // reverse‑pair connections (anti‑diagonal)
    int i = 0;
    while (i < n) {
        int j = n - 1 - i;
        g[i][j] = 1.0f;
        g[j][i] = 1.0f;
        ++i;
    }

    // path edges to increase density
    i = 0;
    while (i < n - 1) {
        g[i][i + 1] = 1.0f;
        g[i + 1][i] = 1.0f;
        ++i;
    }

    vector<float> vals = spectrum(g);
    cout << "Adjacency spectrum (approx.): ";
    out(vals);
    return 0;
}
