/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    // -------------------------------------------------------------
    // Deterministic test vectors – minimal ALBERT‑like forward pass
    // -------------------------------------------------------------
    int seq_len = 1;          // minimal sequence length
    int hidden_sz = 1;        // minimal hidden dimension
    int inter_sz = 1;         // minimal intermediate (FFN) dimension
    float scale = 1.0f / std::sqrt(static_cast<float>(hidden_sz));

    // Input tensor (seq_len * hidden_sz) – simple deterministic value
    std::vector<float> x_input = {0.5f};

    // Weight matrices – minimal deterministic numbers
    // Wq, Wk, Wv, Wo : hidden_sz x hidden_sz
    std::vector<float> w_q = {0.1f};
    std::vector<float> w_k = {0.2f};
    std::vector<float> w_v = {0.3f};
    std::vector<float> w_o = {0.4f};

    // Feed‑forward weights – hidden_sz x inter_sz and inter_sz x hidden_sz
    std::vector<float> w_1 = {0.5f}; // hidden_sz -> inter_sz
    std::vector<float> w_2 = {0.6f}; // inter_sz -> hidden_sz

    // -------------------------------------------------------------
    // 1. Linear projections: Q, K, V
    // -------------------------------------------------------------
    std::vector<float> x_q(seq_len * hidden_sz);
    std::vector<float> x_k(seq_len * hidden_sz);
    std::vector<float> x_v(seq_len * hidden_sz);
    for (int t = 0; t < seq_len; ++t) {
        for (int i = 0; i < hidden_sz; ++i) {
            float q = 0.0f, k = 0.0f, v = 0.0f;
            for (int j = 0; j < hidden_sz; ++j) {
                float inp = x_input[t * hidden_sz + j];
                q += inp * w_q[j * hidden_sz + i];
                k += inp * w_k[j * hidden_sz + i];
                v += inp * w_v[j * hidden_sz + i];
            }
            x_q[t * hidden_sz + i] = q;
            x_k[t * hidden_sz + i] = k;
            x_v[t * hidden_sz + i] = v;
        }
    }

    // -------------------------------------------------------------
    // 2. Scaled dot‑product attention (generic)
    // -------------------------------------------------------------
    std::vector<float> x_score(seq_len * seq_len);
    for (int i = 0; i < seq_len; ++i) {
        for (int j = 0; j < seq_len; ++j) {
            float dot = 0.0f;
            for (int h = 0; h < hidden_sz; ++h) {
                dot += x_q[i * hidden_sz + h] * x_k[j * hidden_sz + h];
            }
            x_score[i * seq_len + j] = dot * scale;
        }
    }

    // Softmax per row
    std::vector<float> x_attn(seq_len * seq_len);
    for (int i = 0; i < seq_len; ++i) {
        float max_val = x_score[i * seq_len];
        for (int j = 1; j < seq_len; ++j) {
            if (x_score[i * seq_len + j] > max_val) max_val = x_score[i * seq_len + j];
        }
        float sum = 0.0f;
        for (int j = 0; j < seq_len; ++j) {
            float e = std::exp(x_score[i * seq_len + j] - max_val);
            x_attn[i * seq_len + j] = e;
            sum += e;
        }
        for (int j = 0; j < seq_len; ++j) {
            x_attn[i * seq_len + j] /= sum;
        }
    }

    // -------------------------------------------------------------
    // 3. Context = Σ softmax * V
    // -------------------------------------------------------------
    std::vector<float> x_ctx(seq_len * hidden_sz);
    for (int i = 0; i < seq_len; ++i) {
        for (int h = 0; h < hidden_sz; ++h) {
            float ctx = 0.0f;
            for (int j = 0; j < seq_len; ++j) {
                ctx += x_attn[i * seq_len + j] * x_v[j * hidden_sz + h];
            }
            x_ctx[i * hidden_sz + h] = ctx;
        }
    }

    // -------------------------------------------------------------
    // 4. Output projection + residual
    // -------------------------------------------------------------
    std::vector<float> x_proj(seq_len * hidden_sz);
    for (int t = 0; t < seq_len; ++t) {
        for (int i = 0; i < hidden_sz; ++i) {
            float out = 0.0f;
            for (int j = 0; j < hidden_sz; ++j) {
                out += x_ctx[t * hidden_sz + j] * w_o[j * hidden_sz + i];
            }
            x_proj[t * hidden_sz + i] = out;
        }
    }

    std::vector<float> x_res1(seq_len * hidden_sz);
    for (int i = 0; i < seq_len * hidden_sz; ++i) {
        x_res1[i] = x_proj[i] + x_input[i];
    }

    // -------------------------------------------------------------
    // 5. Feed‑forward network (hidden -> inter -> hidden) with ReLU
    // -------------------------------------------------------------
    std::vector<float> x_ff1(seq_len * inter_sz);
    for (int t = 0; t < seq_len; ++t) {
        for (int i = 0; i < inter_sz; ++i) {
            float sum = 0.0f;
            for (int j = 0; j < hidden_sz; ++j) {
                sum += x_res1[t * hidden_sz + j] * w_1[j * inter_sz + i];
            }
            x_ff1[t * inter_sz + i] = sum < 0.0f ? 0.0f : sum;
        }
    }

    std::vector<float> x_ff2(seq_len * hidden_sz);
    for (int t = 0; t < seq_len; ++t) {
        for (int i = 0; i < hidden_sz; ++i) {
            float sum = 0.0f;
            for (int j = 0; j < inter_sz; ++j) {
                sum += x_ff1[t * inter_sz + j] * w_2[j * hidden_sz + i];
            }
            x_ff2[t * hidden_sz + i] = sum;
        }
    }

    // Final residual addition
    std::vector<float> x_final(seq_len * hidden_sz);
    for (int i = 0; i < seq_len * hidden_sz; ++i) {
        x_final[i] = x_ff2[i] + x_res1[i];
    }

    // -------------------------------------------------------------
    // 6. Print the resulting hidden states
    // -------------------------------------------------------------
    std::cout << "ALBERT‑like forward output (minimal deterministic test):\n";
    for (int t = 0; t < seq_len; ++t) {
        std::cout << "Token " << t << ": ";
        for (int h = 0; h < hidden_sz; ++h) {
            std::cout << x_final[t * hidden_sz + h] << ' ';
        }
        std::cout << '\n';
    }
    return 0;
}
