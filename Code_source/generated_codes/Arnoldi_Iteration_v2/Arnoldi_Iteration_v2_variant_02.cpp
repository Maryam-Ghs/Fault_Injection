#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>

/* LLM input variant 2: small-diverse */

float dot_product(const std::vector<float>& u, const std::vector<float>& v)
{
    float acc = 0.0f;
    for (int idx = 0; idx < (int)u.size(); ++idx)
        acc += u[idx] * v[idx];
    return acc;
}

void scale_vector(std::vector<float>& v, float alpha)
{
    for (int idx = 0; idx < (int)v.size(); ++idx)
        v[idx] *= alpha;
}

void add_scaled(std::vector<float>& target,
                const std::vector<float>& src,
                float alpha)
{
    for (int idx = 0; idx < (int)target.size(); ++idx)
        target[idx] += alpha * src[idx];
}

class ArnoldiSolver
{
public:
    void run(const std::vector<std::vector<float>>& A,
             const std::vector<float>& r0,
             int m)
    {
        int n = (int)A.size();
        Q.assign(n, std::vector<float>(m + 1, 0.0f));
        H.assign(m + 1, std::vector<float>(m, 0.0f));

        std::vector<float> tmp = r0;
        float norm0 = std::sqrt(dot_product(tmp, tmp));
        for (int i = 0; i < n; ++i)
            Q[i][0] = tmp[i] / norm0;

        for (int step = 0; step < m; ++step)
        {
            std::vector<float> w(n, 0.0f);
            for (int row = 0; row < n; ++row)
            {
                float sum = 0.0f;
                for (int col = 0; col < n; ++col)
                    sum += A[row][col] * Q[col][step];
                w[row] = sum;
            }

            for (int j = 0; j <= step; ++j)
            {
                float proj = dot_product(Q[j], w);
                H[j][step] = proj;
                add_scaled(w, Q[j], -proj);
            }

            float new_norm = std::sqrt(dot_product(w, w));
            H[step + 1][step] = new_norm;
            if (new_norm < 1e-6f) break;

            for (int i = 0; i < n; ++i)
                Q[i][step + 1] = w[i] / new_norm;
        }

        print_basis();
        print_hessenberg();
    }

private:
    std::vector<std::vector<float>> Q;
    std::vector<std::vector<float>> H;

    void print_basis()
    {
        std::cout << "Orthonormal basis Q (columns):\n";
        for (size_t r = 0; r < Q.size(); ++r)
        {
            for (size_t c = 0; c < Q[0].size(); ++c)
                std::cout << std::setw(10) << std::fixed << std::setprecision(4) << Q[r][c];
            std::cout << '\n';
        }
        std::cout << std::endl;
    }

    void print_hessenberg()
    {
        std::cout << "Hessenberg matrix H:\n";
        for (size_t r = 0; r < H.size(); ++r)
        {
            for (size_t c = 0; c < H[0].size(); ++c)
                std::cout << std::setw(10) << std::fixed << std::setprecision(4) << H[r][c];
            std::cout << '\n';
        }
        std::cout << std::endl;
    }
};

int main()
{
    // 3×3 matrix with varied entries
    std::vector<std::vector<float>> A = {
        { 2.5f, -1.2f,  0.0f},
        { 0.7f,  3.3f,  1.4f},
        { -0.5f, 2.1f, -2.2f}
    };

    // Start vector with mixed values
    std::vector<float> b = { 1.0f, -1.0f, 0.5f };

    int m = 3; // full Arnoldi steps for a 3×3 matrix

    ArnoldiSolver solver;
    solver.run(A, b, m);
    return 0;
}
