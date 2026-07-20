/* LLM input variant 6: ordered-structured */
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    // -------------------------------------------------------------
    // Deterministic ordered‑structured test vectors – tiny ALBERT‑like forward pass
    // -------------------------------------------------------------
    const int seq_len = 3;          // sequence length
    const int hidden_sz = 5;        // hidden dimension
    const int inter_sz  = 10;       // intermediate (FFN) dimension
    const float scale = 1.0f / std::sqrt(static_cast<float>(hidden_sz)); // attention scaling

    // Input tensor (seq_len * hidden_sz) – ascending deterministic values
    std::vector<float> x_input(seq_len * hidden_sz);
    for (int i = 0; i < seq_len * hidden_sz; ++i) {
        x_input[i] = 0.1f * static_cast<float>(i + 1); // 0.1, 0.2, ...
    }

    // Weight matrices – ordered patterns
    // Wq, Wk, Wv, Wo : hidden_sz x hidden_sz (identity‑like)
    std::vector<float> w_q(hidden_sz * hidden_sz, 0.0f);
    std::vector<float> w_k(hidden_sz * hidden_sz, 0.0f);
    std::vector<float> w_v(hidden_sz * hidden_sz, 0.0f);
    std::vector<float> w_o(hidden_sz * hidden_sz, 0.0f);
    for (int i = 0; i < hidden_sz; ++i) {
        w_q[i * hidden_sz + i] = 0.1f;
        w_k[i * hidden_sz + i] = 0.1f;
        w_v[i * hidden_sz + i] = 0.1f;
        w_o[i * hidden_sz + i] = 0.1f;
    }

    // Feed‑forward weights – ordered increasing values
    // w_1 : hidden_sz x inter_sz
    // w_2 : inter_sz x hidden_sz
    std::vector<float> w_1(hidden_sz * inter_sz);
    std::vector<float> w_2(inter_sz * hidden_sz);
    for (int i = 0; i < hidden_sz; ++i) {
        for (int j = 0; j < inter_sz; ++j) {
            w_1[i * inter_sz + j] = 0.01f * static_cast<float>(i * inter_sz + j + 1);
        }
    }
    for (int i = 0; i < inter_sz; ++i) {
        for (int j = 0; j < hidden_sz; ++j) {
            w_2[i * hidden_sz + j] = 0.01f * static_cast<float>(i * hidden_sz + j + 1);
        }
    }

    // -------------------------------------------------------------
    // 1. Linear projections: Q, K, V
    // -------------------------------------------------------------
    std::vector<float> x_q(seq_len * hidden_sz, 0.0f);
    std::vector<float> x_k(seq_len * hidden_sz, 0.0f);
    std::vector<float> x_v(seq_len * hidden_sz, 0.0f);

    for (int token = 0; token < seq_len; ++token) {
        int in_off  = token * hidden_sz;
        int out_off = token * hidden_sz;
        for (int i = 0; i < hidden_sz; ++i) {
            float sum_q = 0.0f, sum_k = 0.0f, sum_v = 0.0f;
            for (int j = 0; j < hidden_sz; ++j) {
                float inp = x_input[in_off + j];
                sum_q += inp * w_q[j * hidden_sz + i];
                sum_k += inp * w_k[j * hidden_sz + i];
                sum_v += inp * w_v[j * hidden_sz + i];
            }
            x_q[out_off + i] = sum_q;
            x_k[out_off + i] = sum_k;
            x_v[out_off + i] = sum_v;
        }
    }

    // -------------------------------------------------------------
    // 2. Scaled dot‑product attention
    // -------------------------------------------------------------
    std::vector<float> x_score(seq_len * seq_len, 0.0f);
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
    std::vector<float> x_attn(seq_len * seq_len, 0.0f);
    for (int i = 0; i < seq_len; ++i) {
        float max_val = x_score[i * seq_len];
        for (int j = 1; j < seq_len; ++j) {
            float v = x_score[i * seq_len + j];
            if (v > max_val) max_val = v;
        }
        // subtract max for numerical stability
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
    std::vector<float> x_ctx(seq_len * hidden_sz, 0.0f);
    for (int i = 0; i < seq_len; ++i) {          // output token
        for (int h = 0; h < hidden_sz; ++h) {
            float acc = 0.0f;
            for (int j = 0; j < seq_len; ++j) { // sum over keys
                acc += x_attn[i * seq_len + j] * x_v[j * hidden_sz + h];
            }
            x_ctx[i * hidden_sz + h] = acc;
        }
    }

    // -------------------------------------------------------------
    // 4. Output projection + residual
    // -------------------------------------------------------------
    std::vector<float> x_proj(seq_len * hidden_sz, 0.0f);
    for (int token = 0; token < seq_len; ++token) {
        int in_off  = token * hidden_sz;
        int out_off = token * hidden_sz;
        for (int i = 0; i < hidden_sz; ++i) {
            float sum = 0.0f;
            for (int j = 0; j < hidden_sz; ++j) {
                sum += x_ctx[in_off + j] * w_o[j * hidden_sz + i];
            }
            x_proj[out_off + i] = sum;
        }
    }

    // Residual addition (attention output + original input)
    std::vector<float> x_res1(seq_len * hidden_sz, 0.0f);
    for (int i = 0; i < seq_len * hidden_sz; ++i) {
        x_res1[i] = x_proj[i] + x_input[i];
    }

    // -------------------------------------------------------------
    // 5. Feed‑forward network (hidden -> inter -> hidden) with ReLU
    // -------------------------------------------------------------
    std::vector<float> x_ff1(seq_len * inter_sz, 0.0f);
    // Linear 1
    for (int token = 0; token < seq_len; ++token) {
        int in_off  = token * hidden_sz;
        int out_off = token * inter_sz;
        for (int j = 0; j < inter_sz; ++j) {
            float sum = 0.0f;
            for (int h = 0; h < hidden_sz; ++h) {
                sum += x_res1[in_off + h] * w_1[h * inter_sz + j];
            }
            // ReLU
            x_ff1[out_off + j] = sum > 0.0f ? sum : 0.0f;
        }
    }

    // Linear 2 back to hidden
    std::vector<float> x_ff2(seq_len * hidden_sz, 0.0f);
    for (int token = 0; token < seq_len; ++token) {
        int in_off  = token * inter_sz;
        int out_off = token * hidden_sz;
        for (int j = 0; j < hidden_sz; ++j) {
            float sum = 0.0f;
            for (int h = 0; h < inter_sz; ++h) {
                sum += x_ff1[in_off + h] * w_2[h * hidden_sz + j];
            }
            x_ff2[out_off + j] = sum;
        }
    }

    // Final residual addition
    std::vector<float> x_final(seq_len * hidden_sz, 0.0f);
    for (int i = 0; i < seq_len * hidden_sz; ++i) {
        x_final[i] = x_ff2[i] + x_res1[i];
    }

    // -------------------------------------------------------------
    // 6. Print the resulting hidden states
    // -------------------------------------------------------------
    std::cout << "ALBERT‑like forward output (ordered‑structured test vectors):\n";
    for (int t = 0; t < seq_len; ++t) {
        std::cout << "Token " << t << ": ";
        for (int h = 0; h < hidden_sz; ++h) {
            std::cout << x_final[t * hidden_sz + h] << ' ';
        }
        std::cout << '\n';
    }
    return 0;
}
