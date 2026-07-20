#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// Helper: generate a random float in [-0.5, 0.5]
float rand_float()
{
    int r = std::rand() % 10001;               // 0 .. 10000
    float f = (float)r / 10000.0f - 0.5f;      // [-0.5, 0.5]
    return f;
}

// ------------------------------------------------------------
// Initialise a matrix (rows*cols) with random values
std::vector<float> make_matrix(int rows, int cols)
{
    std::vector<float> mat(rows * cols);
    int i = 0;
    while (i < rows * cols)
    {
        mat[i] = rand_float();
        i = i + 1;
    }
    return mat;
}

// ------------------------------------------------------------
// Matrix multiplication: C = A (aRows x aCols) * B (aCols x bCols)
std::vector<float> mul_mat(const std::vector<float>& A,
                           const std::vector<float>& B,
                           int aRows, int aCols, int bCols)
{
    std::vector<float> C(aRows * bCols);
    int r = 0;
    while (r < aRows)
    {
        int c = 0;
        while (c < bCols)
        {
            float acc = 0.0f;
            int k = 0;
            while (k < aCols)
            {
                acc = acc + A[r * aCols + k] * B[k * bCols + c];
                k = k + 1;
            }
            C[r * bCols + c] = acc;
            c = c + 1;
        }
        r = r + 1;
    }
    return C;
}

// ------------------------------------------------------------
// Transpose a matrix: B = A^T  (rows x cols) -> (cols x rows)
std::vector<float> trans_mat(const std::vector<float>& A, int rows, int cols)
{
    std::vector<float> B(cols * rows);
    int i = 0;
    while (i < rows)
    {
        int j = 0;
        while (j < cols)
        {
            B[j * rows + i] = A[i * cols + j];
            j = j + 1;
        }
        i = i + 1;
    }
    return B;
}

// ------------------------------------------------------------
// Softmax over a vector (in‑place)
void softmax_vec(std::vector<float>& v)
{
    int i = 0;
    float maxv = v[0];
    while (i < (int)v.size())
    {
        if (v[i] > maxv) maxv = v[i];
        i = i + 1;
    }

    float sum = 0.0f;
    i = 0;
    while (i < (int)v.size())
    {
        v[i] = std::exp(v[i] - maxv);
        sum = sum + v[i];
        i = i + 1;
    }

    i = 0;
    while (i < (int)v.size())
    {
        v[i] = v[i] / sum;
        i = i + 1;
    }
}

// ------------------------------------------------------------
// Scaled dot‑product attention for one head
std::vector<float> attend(const std::vector<float>& Q,
                          const std::vector<float>& K,
                          const std::vector<float>& V,
                          int seqLen, int headDim)
{
    // scores = Q * K^T
    std::vector<float> KT = trans_mat(K, seqLen, headDim);
    std::vector<float> scores = mul_mat(Q, KT, seqLen, headDim, seqLen);

    // scale
    float scale = 1.0f / std::sqrt((float)headDim);
    int i = 0;
    while (i < (int)scores.size())
    {
        scores[i] = scores[i] * scale;
        i = i + 1;
    }

    // softmax per row
    int r = 0;
    while (r < seqLen)
    {
        std::vector<float> row(seqLen);
        int c = 0;
        while (c < seqLen)
        {
            row[c] = scores[r * seqLen + c];
            c = c + 1;
        }
        softmax_vec(row);
        c = 0;
        while (c < seqLen)
        {
            scores[r * seqLen + c] = row[c];
            c = c + 1;
        }
        r = r + 1;
    }

    // context = scores * V
    std::vector<float> context = mul_mat(scores, V, seqLen, seqLen, headDim);
    return context;
}

// ------------------------------------------------------------
// Simple feed‑forward network (two linear layers + GELU)
std::vector<float> feed_forward(const std::vector<float>& X,
                                const std::vector<float>& W1,
                                const std::vector<float>& B1,
                                const std::vector<float>& W2,
                                const std::vector<float>& B2,
                                int seqLen, int hidDim, int ffDim)
{
    // X * W1 + B1
    std::vector<float> hidden = mul_mat(X, W1, seqLen, hidDim, ffDim);
    int i = 0;
    while (i < seqLen * ffDim)
    {
        hidden[i] = hidden[i] + B1[i];
        i = i + 1;
    }

    // GELU approximation
    i = 0;
    while (i < seqLen * ffDim)
    {
        float x = hidden[i];
        float y = 0.5f * x * (1.0f + std::tanh(0.79788456f * (x + 0.044715f * x * x * x)));
        hidden[i] = y;
        i = i + 1;
    }

    // hidden * W2 + B2
    std::vector<float> out = mul_mat(hidden, W2, seqLen, ffDim, hidDim);
    i = 0;
    while (i < seqLen * hidDim)
    {
        out[i] = out[i] + B2[i];
        i = i + 1;
    }
    return out;
}

// ------------------------------------------------------------
// Layer normalisation (per token)
void layer_norm(std::vector<float>& X,
                const std::vector<float>& gamma,
                const std::vector<float>& beta,
                int seqLen, int hidDim)
{
    int t = 0;
    while (t < seqLen)
    {
        // mean
        float mean = 0.0f;
        int j = 0;
        while (j < hidDim)
        {
            mean = mean + X[t * hidDim + j];
            j = j + 1;
        }
        mean = mean / (float)hidDim;

        // variance
        float var = 0.0f;
        j = 0;
        while (j < hidDim)
        {
            float diff = X[t * hidDim + j] - mean;
            var = var + diff * diff;
            j = j + 1;
        }
        var = var / (float)hidDim;

        // normalise
        j = 0;
        while (j < hidDim)
        {
            float norm = (X[t * hidDim + j] - mean) / std::sqrt(var + 1e-5f);
            X[t * hidDim + j] = norm * gamma[j] + beta[j];
            j = j + 1;
        }
        t = t + 1;
    }
}

// ------------------------------------------------------------
// One ALBERT transformer layer (parameter‑shared)
void transformer_layer(std::vector<float>& X,
                       const std::vector<float>& Wq,
                       const std::vector<float>& Wk,
                       const std::vector<float>& Wv,
                       const std::vector<float>& Wo,
                       const std::vector<float>& Wff1,
                       const std::vector<float>& Bff1,
                       const std::vector<float>& Wff2,
                       const std::vector<float>& Bff2,
                       const std::vector<float>& gamma1,
                       const std::vector<float>& beta1,
                       const std::vector<float>& gamma2,
                       const std::vector<float>& beta2,
                       int seqLen, int hidDim, int ffDim)
{
    // ---- Multi‑head (single head) attention ----
    std::vector<float> Q = mul_mat(X, Wq, seqLen, hidDim, hidDim);
    std::vector<float> K = mul_mat(X, Wk, seqLen, hidDim, hidDim);
    std::vector<float> V = mul_mat(X, Wv, seqLen, hidDim, hidDim);

    std::vector<float> att_out = attend(Q, K, V, seqLen, hidDim);
    std::vector<float> proj = mul_mat(att_out, Wo, seqLen, hidDim, hidDim);

    // Residual + LayerNorm 1
    int i = 0;
    while (i < seqLen * hidDim)
    {
        X[i] = X[i] + proj[i];
        i = i + 1;
    }
    layer_norm(X, gamma1, beta1, seqLen, hidDim);

    // ---- Feed‑forward ----
    std::vector<float> ff = feed_forward(X, Wff1, Bff1, Wff2, Bff2,
                                         seqLen, hidDim, ffDim);

    // Residual + LayerNorm 2
    i = 0;
    while (i < seqLen * hidDim)
    {
        X[i] = X[i] + ff[i];
        i = i + 1;
    }
    layer_norm(X, gamma2, beta2, seqLen, hidDim);
}

// ------------------------------------------------------------
int main()
{
    std::srand((unsigned)std::time(0));

    // Model dimensions (large safe stress)
    int seqLen = 256;     // sequence length
    int hidDim = 256;    // hidden size
    int ffDim  = 1024;    // feed‑forward inner size

    // Random input embeddings
    std::vector<float> input = make_matrix(seqLen, hidDim);

    // Shared weights (ALBERT style)
    std::vector<float> Wq   = make_matrix(hidDim, hidDim);
    std::vector<float> Wk   = make_matrix(hidDim, hidDim);
    std::vector<float> Wv   = make_matrix(hidDim, hidDim);
    std::vector<float> Wo   = make_matrix(hidDim, hidDim);
    std::vector<float> Wff1 = make_matrix(hidDim, ffDim);
    std::vector<float> Bff1 = make_matrix(1, ffDim);
    std::vector<float> Wff2 = make_matrix(ffDim, hidDim);
    std::vector<float> Bff2 = make_matrix(1, hidDim);

    // Layer‑norm parameters
    std::vector<float> gamma1 = make_matrix(1, hidDim);
    std::vector<float> beta1  = make_matrix(1, hidDim);
    std::vector<float> gamma2 = make_matrix(1, hidDim);
    std::vector<float> beta2  = make_matrix(1, hidDim);

    // Run a single transformer layer
    transformer_layer(input, Wq, Wk, Wv, Wo,
                      Wff1, Bff1, Wff2, Bff2,
                      gamma1, beta1, gamma2, beta2,
                      seqLen, hidDim, ffDim);

    // Print the resulting matrix (seqLen x hidDim)
    int r = 0;
    while (r < seqLen)
    {
        std::cout << "Token " << r << ": ";
        int c = 0;
        while (c < hidDim)
        {
            std::cout << input[r * hidDim + c] << ' ';
            c = c + 1;
        }
        std::cout << std::endl;
        r = r + 1;
    }

    return 0;
}
