/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <cmath>
#include <iomanip>

// ------------------------------------------------------------
// Configuration (no const, only macros)
// ------------------------------------------------------------
#define VOCAB_SIZE  1000
#define EMBED_DIM   32
#define HIDDEN_DIM  32
#define MAX_SEQ     10
#define EPS         1e-5f

// ------------------------------------------------------------
// Simple ALBERT‑like transformer (single block, stack memory)
// ------------------------------------------------------------
class SimpleAlbert {
public:
    // Weight tensors (stack allocated inside the object)
    float embed[VOCAB_SIZE][EMBED_DIM];
    float w_q[EMBED_DIM][HIDDEN_DIM];
    float w_k[EMBED_DIM][HIDDEN_DIM];
    float w_v[EMBED_DIM][HIDDEN_DIM];
    float w_o[HIDDEN_DIM][EMBED_DIM];
    float w_ff1[EMBED_DIM][HIDDEN_DIM];
    float w_ff2[HIDDEN_DIM][EMBED_DIM];
    float ln_gamma[EMBED_DIM];
    float ln_beta[EMBED_DIM];

    // --------------------------------------------------------
    // Fill all parameters with deterministic pseudo‑random numbers
    // --------------------------------------------------------
    void init_random() {
        // Helper lambda to produce a deterministic value in [-0.5, 0.5)
        auto gen = [](int a, int b) -> float {
            const int prime = 997; // prime for good distribution
            int val = (a * 37 + b * 23) % prime;
            return static_cast<float>(val) / static_cast<float>(prime) - 0.5f;
        };

        for (int i = 0; i < VOCAB_SIZE; ++i)
            for (int j = 0; j < EMBED_DIM; ++j)
                embed[i][j] = gen(i, j);

        for (int i = 0; i < EMBED_DIM; ++i)
            for (int j = 0; j < HIDDEN_DIM; ++j) {
                w_q[i][j]   = gen(i, j + 100);
                w_k[i][j]   = gen(i + 200, j);
                w_v[i][j]   = gen(i + 400, j + 300);
                w_ff1[i][j] = gen(i + 600, j + 500);
            }

        for (int i = 0; i < HIDDEN_DIM; ++i)
            for (int j = 0; j < EMBED_DIM; ++j) {
                w_o[i][j]   = gen(i + 800, j + 700);
                w_ff2[i][j] = gen(i + 900, j + 850);
            }

        for (int i = 0; i < EMBED_DIM; ++i) {
            ln_gamma[i] = 1.0f;
            ln_beta[i]  = 0.0f;
        }
    }

    // --------------------------------------------------------
    // Forward pass for a fixed‑size sequence (MAX_SEQ)
    // --------------------------------------------------------
    void forward(int *ids, int seq_len, float *out) {
        // ---------- Embedding ----------
        float hidden[MAX_SEQ][EMBED_DIM];
        for (int s = 0; s < seq_len; ++s)
            for (int d = 0; d < EMBED_DIM; ++d) {
                int idx = ids[s];
                hidden[s][d] = embed[idx][d];
            }

        // ---------- Linear projections ----------
        float q[MAX_SEQ][HIDDEN_DIM];
        float k[MAX_SEQ][HIDDEN_DIM];
        float v[MAX_SEQ][HIDDEN_DIM];
        for (int s = 0; s < seq_len; ++s)
            for (int d = 0; d < HIDDEN_DIM; ++d) {
                float acc_q = 0.0f, acc_k = 0.0f, acc_v = 0.0f;
                for (int e = 0; e < EMBED_DIM; ++e) {
                    float x = hidden[s][e];
                    acc_q += x * w_q[e][d];
                    acc_k += x * w_k[e][d];
                    acc_v += x * w_v[e][d];
                }
                q[s][d] = acc_q;
                k[s][d] = acc_k;
                v[s][d] = acc_v;
            }

        // ---------- Scaled dot‑product attention ----------
        float scale = 1.0f / std::sqrt(static_cast<float>(HIDDEN_DIM));
        float scores[MAX_SEQ][MAX_SEQ];
        for (int i = 0; i < seq_len; ++i)
            for (int j = 0; j < seq_len; ++j) {
                float sum = 0.0f;
                for (int d = 0; d < HIDDEN_DIM; ++d)
                    sum += q[i][d] * k[j][d];
                scores[i][j] = sum * scale;
            }

        // ---------- Softmax ----------
        float probs[MAX_SEQ][MAX_SEQ];
        for (int i = 0; i < seq_len; ++i) {
            float maxv = scores[i][0];
            for (int j = 1; j < seq_len; ++j)
                maxv = (scores[i][j] > maxv) ? scores[i][j] : maxv;

            float sumexp = 0.0f;
            for (int j = 0; j < seq_len; ++j) {
                float e = std::exp(scores[i][j] - maxv);
                probs[i][j] = e;
                sumexp += e;
            }
            for (int j = 0; j < seq_len; ++j)
                probs[i][j] /= sumexp;
        }

        // ---------- Context ----------
        float ctx[MAX_SEQ][HIDDEN_DIM];
        for (int i = 0; i < seq_len; ++i)
            for (int d = 0; d < HIDDEN_DIM; ++d) {
                float acc = 0.0f;
                for (int j = 0; j < seq_len; ++j)
                    acc += probs[i][j] * v[j][d];
                ctx[i][d] = acc;
            }

        // ---------- Output projection ----------
        float att_out[MAX_SEQ][EMBED_DIM];
        for (int s = 0; s < seq_len; ++s)
            for (int d = 0; d < EMBED_DIM; ++d) {
                float acc = 0.0f;
                for (int h = 0; h < HIDDEN_DIM; ++h)
                    acc += ctx[s][h] * w_o[h][d];
                att_out[s][d] = acc;
            }

        // ---------- Add & LayerNorm (first) ----------
        float ln1[MAX_SEQ][EMBED_DIM];
        for (int s = 0; s < seq_len; ++s) {
            // residual
            for (int d = 0; d < EMBED_DIM; ++d)
                att_out[s][d] += hidden[s][d];

            // mean
            float mean = 0.0f;
            for (int d = 0; d < EMBED_DIM; ++d)
                mean += att_out[s][d];
            mean /= EMBED_DIM;

            // variance
            float var = 0.0f;
            for (int d = 0; d < EMBED_DIM; ++d) {
                float diff = att_out[s][d] - mean;
                var += diff * diff;
            }
            var /= EMBED_DIM;

            // normalize
            float inv = 1.0f / std::sqrt(var + EPS);
            for (int d = 0; d < EMBED_DIM; ++d) {
                float norm = (att_out[s][d] - mean) * inv;
                ln1[s][d] = norm * ln_gamma[d] + ln_beta[d];
            }
        }

        // ---------- Feed‑forward ----------
        float ff1[MAX_SEQ][HIDDEN_DIM];
        for (int s = 0; s < seq_len; ++s)
            for (int d = 0; d < HIDDEN_DIM; ++d) {
                float acc = 0.0f;
                for (int e = 0; e < EMBED_DIM; ++e)
                    acc += ln1[s][e] * w_ff1[e][d];
                // GELU approximation
                float x = acc;
                float gelu = 0.5f * x * (1.0f + std::tanh(std::sqrt(2.0f / 3.14159265f) *
                                 (x + 0.044715f * x * x * x)));
                ff1[s][d] = gelu;
            }

        float ff2[MAX_SEQ][EMBED_DIM];
        for (int s = 0; s < seq_len; ++s)
            for (int d = 0; d < EMBED_DIM; ++d) {
                float acc = 0.0f;
                for (int h = 0; h < HIDDEN_DIM; ++h)
                    acc += ff1[s][h] * w_ff2[h][d];
                ff2[s][d] = acc;
            }

        // ---------- Add & LayerNorm (second) ----------
        for (int s = 0; s < seq_len; ++s) {
            // residual
            for (int d = 0; d < EMBED_DIM; ++d)
                ff2[s][d] += ln1[s][d];

            // mean
            float mean = 0.0f;
            for (int d = 0; d < EMBED_DIM; ++d)
                mean += ff2[s][d];
            mean /= EMBED_DIM;

            // variance
            float var = 0.0f;
            for (int d = 0; d < EMBED_DIM; ++d) {
                float diff = ff2[s][d] - mean;
                var += diff * diff;
            }
            var /= EMBED_DIM;

            // normalize and write to out
            float inv = 1.0f / std::sqrt(var + EPS);
            for (int d = 0; d < EMBED_DIM; ++d) {
                float norm = (ff2[s][d] - mean) * inv;
                out[s * EMBED_DIM + d] = norm * ln_gamma[d] + ln_beta[d];
            }
        }
    }
};

// ------------------------------------------------------------
// Main – generate deterministic token ids, run model, print result
// ------------------------------------------------------------
int main() {
    const int seq_len = MAX_SEQ;                 // fixed length for this demo

    // Deterministic pseudo‑random token ids using a simple LCG
    int ids[MAX_SEQ];
    unsigned int seed = 12345u;
    for (int i = 0; i < seq_len; ++i) {
        seed = (seed * 1103515245u + 12345u) & 0x7fffffff;
        ids[i] = static_cast<int>(seed % VOCAB_SIZE);
    }

    // Model instance on stack
    SimpleAlbert model;
    model.init_random();

    // Output buffer on stack
    float result[MAX_SEQ * EMBED_DIM];
    model.forward(ids, seq_len, result);

    // Print first token representation
    std::cout << "First token output vector:\n";
    for (int d = 0; d < EMBED_DIM; ++d) {
        std::cout << std::fixed << std::setprecision(5) << result[d] << " ";
        if ((d + 1) % 8 == 0) std::cout << "\n";
    }
    std::cout << std::endl;
    return 0;
}
