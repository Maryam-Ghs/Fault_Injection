#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 10: large-safe-stress */
int main() {
    // problem size
    int N = 100;               // matrix dimension
    int M = 50;                // Arnoldi steps (<= N)

    // deterministic matrix A (row‑major) – original formula scaled for size
    std::vector<float> A_mat(N * N);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            A_mat[i * N + j] = static_cast<float>((i + 1) * (j + 2));

    // deterministic start vector b
    std::vector<float> b(N);
    for (int i = 0; i < N; ++i) b[i] = static_cast<float>(i + 1);

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

        // orthogonalisation against previous Q columns
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

        // compute norm of w
        float nxt = 0.0f;
        for (int k = 0; k < N; ++k) nxt += w[k] * w[k];
        nxt = sqrtf(nxt);
        H[(step + 1) * M + step] = nxt;

        // normalise w to obtain next q
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
