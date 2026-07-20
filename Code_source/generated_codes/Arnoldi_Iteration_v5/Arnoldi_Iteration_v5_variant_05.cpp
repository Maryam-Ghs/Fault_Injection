#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */
int main() {
    // problem size
    int N = 4;               // matrix dimension
    int M = 3;               // Arnoldi steps (<= N)

    // deterministic matrix A (row‑major) – all entries identical
    std::vector<float> A_mat(N * N, 5.0f);

    // deterministic start vector b – all entries identical
    std::vector<float> b(N, 2.0f);

    // storage for orthonormal basis Q (N × (M+1)) and Hessenberg H ((M+1) × M)
    std::vector<float> Q(N * (M + 1), 0.0f);
    std::vector<float> H((M + 1) * M, 0.0f);

    // ----- initialise first Arnoldi vector -----
    float nrm = 0.0f;
    for (int i = 0; i < N; ++i) nrm += b[i] * b[i];
    nrm = sqrtf(nrm);
    float inv_nrm = 1.0f / nrm;
    for (int i = 0; i < N; ++i) Q[i] = b[i] * inv_nrm;

    // ----- Arnoldi iteration -----
    int step = 0;
    while (step < M) {
        std::vector<float> w(N, 0.0f);
        for (int row = 0; row < N; ++row) {
            float acc = 0.0f;
            for (int col = 0; col < N; ++col)
                acc += A_mat[row * N + col] * Q[col * (M + 1) + step];
            w[row] = acc;
        }

        int i = 0;
        while (i <= step) {
            float hij = 0.0f;
            for (int k = 0; k < N; ++k)
                hij += Q[k * (M + 1) + i] * w[k];
            H[i * M + step] = hij;

            float coeff = hij;
            for (int k = 0; k < N; ++k)
                w[k] -= coeff * Q[k * (M + 1) + i];
            ++i;
        }

        float nxt = 0.0f;
        for (int k = 0; k < N; ++k) nxt += w[k] * w[k];
        nxt = sqrtf(nxt);
        H[(step + 1) * M + step] = nxt;

        float scale = 1.0f / nxt;
        for (int k = 0; k < N; ++k)
            Q[k * (M + 1) + (step + 1)] = w[k] * scale;

        ++step;
    }

    // ----- output results -----
    std::cout << "Orthonormal basis Q (columns):\n";
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c <= M; ++c)
            std::cout << Q[r * (M + 1) + c] << ' ';
        std::cout << '\n';
    }

    std::cout << "\nUpper Hessenberg matrix H:\n";
    for (int r = 0; r <= M; ++r) {
        for (int c = 0; c < M; ++c)
            std::cout << H[r * M + c] << ' ';
        std::cout << '\n';
    }
    return 0;
}
