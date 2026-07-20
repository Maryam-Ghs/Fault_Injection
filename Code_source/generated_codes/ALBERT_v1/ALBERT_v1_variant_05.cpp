#include <iostream>
#include <cstdlib>
#include <cmath>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */

// ------------------------------------------------------------
// Configuration (no const, only macros)
// ------------------------------------------------------------
#define VOCAB_SIZE  1000
#define EMBED_DIM   32
#define HIDDEN_DIM  32
#define MAX_SEQ     8
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
    // Fill all parameters with small random numbers
    // --------------------------------------------------------
    void init_random() {
        for (int i = 0; i < VOCAB_SIZE; ++i)
            for (int j = 0; j < EMBED_DIM; ++j)
                embed[i][j] = (float)std::rand() / RAND_MAX - 0.5f;

        for (int i = 0; i < EMBED_DIM; ++i)
            for (int j = 0; j < HIDDEN_DIM; ++j) {
                w_q[i][j] = (float)std::rand() / RAND_MAX - 0.5f;
                w_k[i][j] = (float)std::rand() / RAND_MAX - 0.5f;
                w_v[i][j] = (float)std::rand() / RAND_MAX - 0.5f;
                w_ff1[i][j] = (float)std::rand() / RAND_MAX - 0.5f;
            }

        for (int i = 0; i < HIDDEN_DIM; ++i)
            for (int j = 0; j < EMBED_DIM; ++j) {
                w_o[i][j] = (float)std::rand() / RAND_MAX - 0.5f;
                w_ff2[i][j] = (float)std::rand() / RAND_MAX - 0.5f;
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
        float scale = 1.0f / std::sqrt((float)HIDDEN_DIM);
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
// Main – generate duplicate‑heavy token ids, run model, print result
// ------------------------------------------------------------
int main() {
    std::srand(12345);                     // deterministic randomness for weights
    int seq_len = MAX_SEQ;                 // fixed length for this demo

    // Duplicate‑heavy token ids: repeat the same token index across the sequence
    const int duplicate_token = 42;        // arbitrary token within VOCAB_SIZE
    int ids[MAX_SEQ];
    for (int i = 0; i < seq_len; ++i)
        ids[i] = duplicate_token;

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
