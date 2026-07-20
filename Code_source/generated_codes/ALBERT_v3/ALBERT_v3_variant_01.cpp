/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <cmath>
#include <algorithm>

#define VOCAB_SIZE 2
#define SEQ_LEN    2
#define HIDDEN_DIM 2
#define HEADS      1
#define HEAD_DIM   (HIDDEN_DIM / HEADS)

// ------------------------------------------------------------
// Helper: simple layer‑norm (no epsilon handling for brevity)
// ------------------------------------------------------------
void normLayer(float out[SEQ_LEN][HIDDEN_DIM],
               const float in[SEQ_LEN][HIDDEN_DIM])
{
    int i = 0;
    while (i < SEQ_LEN) {
        float mean = 0.0f;
        int j = 0;
        while (j < HIDDEN_DIM) {
            mean += in[i][j];
            ++j;
        }
        mean /= (float)HIDDEN_DIM;

        float var = 0.0f;
        j = 0;
        while (j < HIDDEN_DIM) {
            float diff = in[i][j] - mean;
            var += diff * diff;
            ++j;
        }
        var /= (float)HIDDEN_DIM;
        float stddev = std::sqrt(var);

        j = 0;
        while (j < HIDDEN_DIM) {
            out[i][j] = (in[i][j] - mean) / stddev;
            ++j;
        }
        ++i;
    }
}

// ------------------------------------------------------------
// Helper: matrix multiplication (A: m×k, B: k×n, C: m×n)
// ------------------------------------------------------------
void mulMat(float C[SEQ_LEN][HIDDEN_DIM],
            const float A[SEQ_LEN][HIDDEN_DIM],
            const float B[HIDDEN_DIM][HIDDEN_DIM])
{
    int r = 0;
    while (r < SEQ_LEN) {
        int c = 0;
        while (c < HIDDEN_DIM) {
            float acc = 0.0f;
            int k = 0;
            while (k < HIDDEN_DIM) {
                acc += A[r][k] * B[k][c];
                ++k;
            }
            C[r][c] = acc;
            ++c;
        }
        ++r;
    }
}

// ------------------------------------------------------------
// Helper: element‑wise addition (in‑place)
// ------------------------------------------------------------
void addInPlace(float target[SEQ_LEN][HIDDEN_DIM],
                const float addend[SEQ_LEN][HIDDEN_DIM])
{
    int i = 0;
    while (i < SEQ_LEN) {
        int j = 0;
        while (j < HIDDEN_DIM) {
            target[i][j] += addend[i][j];
            ++j;
        }
        ++i;
    }
}

// ------------------------------------------------------------
// Helper: feed‑forward network (two linear layers + GELU)
// ------------------------------------------------------------
void ffnBlock(float out[SEQ_LEN][HIDDEN_DIM],
              const float in[SEQ_LEN][HIDDEN_DIM],
              const float w1[HIDDEN_DIM][HIDDEN_DIM],
              const float w2[HIDDEN_DIM][HIDDEN_DIM])
{
    float tmp[SEQ_LEN][HIDDEN_DIM];
    mulMat(tmp, in, w1);

    // GELU approximation
    int i = 0;
    while (i < SEQ_LEN) {
        int j = 0;
        while (j < HIDDEN_DIM) {
            float x = tmp[i][j];
            float x3 = x * x * x;
            float tanh_arg = 0.7978845608f * (x + 0.044715f * x3);
            tmp[i][j] = 0.5f * x * (1.0f + std::tanh(tanh_arg));
            ++j;
        }
        ++i;
    }

    mulMat(out, tmp, w2);
}

// ------------------------------------------------------------
// Helper: scaled dot‑product attention for a single head
// ------------------------------------------------------------
void headAttention(float out[SEQ_LEN][HEAD_DIM],
                   const float q[SEQ_LEN][HEAD_DIM],
                   const float k[SEQ_LEN][HEAD_DIM],
                   const float v[SEQ_LEN][HEAD_DIM])
{
    // Compute scores = Q * K^T
    float scores[SEQ_LEN][SEQ_LEN];
    int i = 0;
    while (i < SEQ_LEN) {
        int j = 0;
        while (j < SEQ_LEN) {
            float acc = 0.0f;
            int d = 0;
            while (d < HEAD_DIM) {
                acc += q[i][d] * k[j][d];
                ++d;
            }
            scores[i][j] = acc / std::sqrt((float)HEAD_DIM);
            ++j;
        }
        ++i;
    }

    // Softmax rows
    i = 0;
    while (i < SEQ_LEN) {
        float maxv = scores[i][0];
        int j = 1;
        while (j < SEQ_LEN) {
            if (scores[i][j] > maxv) maxv = scores[i][j];
            ++j;
        }

        float sum = 0.0f;
        j = 0;
        while (j < SEQ_LEN) {
            scores[i][j] = std::exp(scores[i][j] - maxv);
            sum += scores[i][j];
            ++j;
        }

        j = 0;
        while (j < SEQ_LEN) {
            scores[i][j] /= sum;
            ++j;
        }
        ++i;
    }

    // Weighted sum: out = scores * V
    i = 0;
    while (i < SEQ_LEN) {
        int d = 0;
        while (d < HEAD_DIM) {
            float acc = 0.0f;
            int j = 0;
            while (j < SEQ_LEN) {
                acc += scores[i][j] * v[j][d];
                ++j;
            }
            out[i][d] = acc;
            ++d;
        }
        ++i;
    }
}

// ------------------------------------------------------------
// Helper: multi‑head attention (concatenates heads)
// ------------------------------------------------------------
void multiHeadAttn(float out[SEQ_LEN][HIDDEN_DIM],
                   const float in[SEQ_LEN][HIDDEN_DIM],
                   const float wq[HIDDEN_DIM][HIDDEN_DIM],
                   const float wk[HIDDEN_DIM][HIDDEN_DIM],
                   const float wv[HIDDEN_DIM][HIDDEN_DIM],
                   const float wo[HIDDEN_DIM][HIDDEN_DIM])
{
    // Project Q,K,V
    float q_all[SEQ_LEN][HIDDEN_DIM];
    float k_all[SEQ_LEN][HIDDEN_DIM];
    float v_all[SEQ_LEN][HIDDEN_DIM];
    mulMat(q_all, in, wq);
    mulMat(k_all, in, wk);
    mulMat(v_all, in, wv);

    // Split heads and compute each head
    float head_res[HEADS][SEQ_LEN][HEAD_DIM];
    int h = 0;
    while (h < HEADS) {
        float qh[SEQ_LEN][HEAD_DIM];
        float kh[SEQ_LEN][HEAD_DIM];
        float vh[SEQ_LEN][HEAD_DIM];

        int i = 0;
        while (i < SEQ_LEN) {
            int d = 0;
            while (d < HEAD_DIM) {
                qh[i][d] = q_all[i][h * HEAD_DIM + d];
                kh[i][d] = k_all[i][h * HEAD_DIM + d];
                vh[i][d] = v_all[i][h * HEAD_DIM + d];
                ++d;
            }
            ++i;
        }

        headAttention(head_res[h], qh, kh, vh);
        ++h;
    }

    // Concatenate heads back into a single matrix
    float concat[SEQ_LEN][HIDDEN_DIM];
    int i = 0;
    while (i < SEQ_LEN) {
        int d = 0;
        while (d < HIDDEN_DIM) {
            int h_idx = d / HEAD_DIM;
            int inner = d % HEAD_DIM;
            concat[i][d] = head_res[h_idx][i][inner];
            ++d;
        }
        ++i;
    }

    // Final linear projection
    mulMat(out, concat, wo);
}

// ------------------------------------------------------------
// Helper: token embedding lookup (stack‑based tables)
// ------------------------------------------------------------
void embedTokens(float embed_out[SEQ_LEN][HIDDEN_DIM],
                 const int ids[SEQ_LEN],
                 const float table[VOCAB_SIZE][HIDDEN_DIM])
{
    int pos = 0;
    while (pos < SEQ_LEN) {
        int id = ids[pos];
        if (id < 0) id = 0;
        if (id >= VOCAB_SIZE) id = VOCAB_SIZE - 1;
        int d = 0;
        while (d < HIDDEN_DIM) {
            embed_out[pos][d] = table[id][d];
            ++d;
        }
        ++pos;
    }
}

// ------------------------------------------------------------
// Main: tiny ALBERT‑like forward pass
// ------------------------------------------------------------
int main()
{
    int token_seq[SEQ_LEN] = {0, -5};

    float embed_table[VOCAB_SIZE][HIDDEN_DIM];
    int vi = 0;
    while (vi < VOCAB_SIZE) {
        int dj = 0;
        while (dj < HIDDEN_DIM) {
            embed_table[vi][dj] = ((vi + dj) % 2 == 0) ? 1.5f * (vi + 1) : -2.3f * (dj + 1);
            ++dj;
        }
        ++vi;
    }

    float w_q[HIDDEN_DIM][HIDDEN_DIM];
    float w_k[HIDDEN_DIM][HIDDEN_DIM];
    float w_v[HIDDEN_DIM][HIDDEN_DIM];
    float w_o[HIDDEN_DIM][HIDDEN_DIM];
    float w_ff1[HIDDEN_DIM][HIDDEN_DIM];
    float w_ff2[HIDDEN_DIM][HIDDEN_DIM];

    int r = 0;
    while (r < HIDDEN_DIM) {
        int c = 0;
        while (c < HIDDEN_DIM) {
            w_q[r][c]  = 0.01f * (r - c);
            w_k[r][c]  = -0.01f * (r + c);
            w_v[r][c]  = 0.02f * (r * c);
            w_o[r][c]  = 0.015f * (r - 2 * c);
            w_ff1[r][c] = 0.005f * (r + 3 * c);
            w_ff2[r][c] = -0.005f * (2 * r - c);
            ++c;
        }
        ++r;
    }

    float hidden[SEQ_LEN][HIDDEN_DIM];
    embedTokens(hidden, token_seq, embed_table);

    float attn_out[SEQ_LEN][HIDDEN_DIM];
    multiHeadAttn(attn_out, hidden, w_q, w_k, w_v, w_o);
    addInPlace(attn_out, hidden);
    float norm1[SEQ_LEN][HIDDEN_DIM];
    normLayer(norm1, attn_out);

    float ffn_out[SEQ_LEN][HIDDEN_DIM];
    ffnBlock(ffn_out, norm1, w_ff1, w_ff2);
    addInPlace(ffn_out, norm1);
    float final_norm[SEQ_LEN][HIDDEN_DIM];
    normLayer(final_norm, ffn_out);

    int i = 0;
    while (i < SEQ_LEN) {
        std::cout << "Token " << i << " (id=" << token_seq[i] << "): ";
        int j = 0;
        while (j < HIDDEN_DIM) {
            std::cout << final_norm[i][j];
            if (j + 1 < HIDDEN_DIM) std::cout << ", ";
            ++j;
        }
        std::cout << std::endl;
        ++i;
    }

    return 0;
}
