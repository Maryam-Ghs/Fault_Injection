#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 4: signed-extremes */
int main() {
    // -------------------------------------------------------------
    // Deterministic test vectors – tiny ALBERT‑like forward pass
    // -------------------------------------------------------------
    int seq_len = 2;          // sequence length
    int hidden_sz = 4;        // hidden dimension
    int inter_sz  = 8;        // intermediate (FFN) dimension
    float scale = 0.5f;       // 1/sqrt(hidden_sz) for attention scaling

    // Input tensor (seq_len * hidden_sz) – mixed negative, zero, positive values
    std::vector<float> x_input = {
        -0.3f, 0.0f, 0.2f, -0.5f,   // token 0
        0.0f, 0.4f, -0.1f, 0.0f    // token 1
    };

    // Weight matrices – mixed signs
    // Wq, Wk, Wv, Wo : hidden_sz x hidden_sz
    std::vector<float> w_q = {
        0.1f, -0.2f, 0.0f, 0.3f,
        -0.1f, 0.0f, 0.2f, -0.3f,
        0.0f, 0.1f, -0.2f, 0.0f,
        0.3f, -0.1f, 0.0f, 0.2f
    };
    std::vector<float> w_k = {
        0.0f, 0.1f, -0.2f, 0.0f,
        0.2f, -0.1f, 0.0f, 0.3f,
        -0.3f, 0.0f, 0.1f, -0.2f,
        0.0f, 0.2f, -0.1f, 0.0f
    };
    std::vector<float> w_v = {
        0.1f, 0.0f, -0.2f, 0.0f,
        0.0f, 0.1f, 0.0f, -0.3f,
        -0.1f, 0.0f, 0.2f, 0.0f,
        0.0f, -0.2f, 0.0f, 0.1f
    };
    std::vector<float> w_o = {
        0.0f, -0.1f, 0.2f, 0.0f,
        0.1f, 0.0f, -0.2f, 0.0f,
        0.0f, 0.2f, 0.0f, -0.1f,
        -0.3f, 0.0f, 0.1f, 0.0f
    };

    // Feed‑forward weights – hidden_sz x inter_sz and inter_sz x hidden_sz
    std::vector<float> w_1 = {
        // row 0
        0.1f, -0.1f, 0.0f, 0.2f, -0.2f, 0.0f, 0.1f, -0.1f,
        // row 1
        0.0f, 0.1f, -0.1f, 0.0f, 0.2f, -0.2f, 0.0f, 0.1f,
        // row 2
        -0.1f, 0.0f, 0.1f, 0.0f, -0.2f, 0.2f, 0.0f, -0.1f,
        // row 3
        0.2f, 0.0f, -0.2f, 0.1f, 0.0f, -0.1f, 0.2f, 0.0f
    };
    std::vector<float> w_2 = {
        // row 0
        0.1f, 0.0f, -0.1f, 0.2f,
        // row 1
        -0.2f, 0.1f, 0.0f, -0.1f,
        // row 2
        0.0f, 0.2f, -0.2f, 0.0f,
        // row 3
        0.1f, -0.1f, 0.0f, 0.2f,
        // row 4
        -0.1f, 0.0f, 0.1f, -0.2f,
        // row 5
        0.2f, -0.2f, 0.1f, 0.0f,
        // row 6
        0.0f, 0.1f, -0.1f, 0.0f,
        // row 7
        -0.2f, 0.0f, 0.2f, -0.1f
    };

    // -------------------------------------------------------------
    // 1. Linear projections: Q, K, V  (manual loop unrolling)
    // -------------------------------------------------------------
    std::vector<float> x_q(seq_len * hidden_sz);
    std::vector<float> x_k(seq_len * hidden_sz);
    std::vector<float> x_v(seq_len * hidden_sz);

    // token 0
    {
        int base = 0;
        // Q0 = input0 * w_q
        x_q[base+0] = x_input[0]*w_q[0] + x_input[1]*w_q[4] + x_input[2]*w_q[8] + x_input[3]*w_q[12];
        x_q[base+1] = x_input[0]*w_q[1] + x_input[1]*w_q[5] + x_input[2]*w_q[9] + x_input[3]*w_q[13];
        x_q[base+2] = x_input[0]*w_q[2] + x_input[1]*w_q[6] + x_input[2]*w_q[10]+ x_input[3]*w_q[14];
        x_q[base+3] = x_input[0]*w_q[3] + x_input[1]*w_q[7] + x_input[2]*w_q[11]+ x_input[3]*w_q[15];

        // K0 = input0 * w_k
        x_k[base+0] = x_input[0]*w_k[0] + x_input[1]*w_k[4] + x_input[2]*w_k[8] + x_input[3]*w_k[12];
        x_k[base+1] = x_input[0]*w_k[1] + x_input[1]*w_k[5] + x_input[2]*w_k[9] + x_input[3]*w_k[13];
        x_k[base+2] = x_input[0]*w_k[2] + x_input[1]*w_k[6] + x_input[2]*w_k[10]+ x_input[3]*w_k[14];
        x_k[base+3] = x_input[0]*w_k[3] + x_input[1]*w_k[7] + x_input[2]*w_k[11]+ x_input[3]*w_k[15];

        // V0 = input0 * w_v
        x_v[base+0] = x_input[0]*w_v[0] + x_input[1]*w_v[4] + x_input[2]*w_v[8] + x_input[3]*w_v[12];
        x_v[base+1] = x_input[0]*w_v[1] + x_input[1]*w_v[5] + x_input[2]*w_v[9] + x_input[3]*w_v[13];
        x_v[base+2] = x_input[0]*w_v[2] + x_input[1]*w_v[6] + x_input[2]*w_v[10]+ x_input[3]*w_v[14];
        x_v[base+3] = x_input[0]*w_v[3] + x_input[1]*w_v[7] + x_input[2]*w_v[11]+ x_input[3]*w_v[15];
    }
    // token 1
    {
        int base = hidden_sz;
        int off  = hidden_sz; // offset into input
        // Q1
        x_q[base+0] = x_input[off+0]*w_q[0] + x_input[off+1]*w_q[4] + x_input[off+2]*w_q[8] + x_input[off+3]*w_q[12];
        x_q[base+1] = x_input[off+0]*w_q[1] + x_input[off+1]*w_q[5] + x_input[off+2]*w_q[9] + x_input[off+3]*w_q[13];
        x_q[base+2] = x_input[off+0]*w_q[2] + x_input[off+1]*w_q[6] + x_input[off+2]*w_q[10]+ x_input[off+3]*w_q[14];
        x_q[base+3] = x_input[off+0]*w_q[3] + x_input[off+1]*w_q[7] + x_input[off+2]*w_q[11]+ x_input[off+3]*w_q[15];

        // K1
        x_k[base+0] = x_input[off+0]*w_k[0] + x_input[off+1]*w_k[4] + x_input[off+2]*w_k[8] + x_input[off+3]*w_k[12];
        x_k[base+1] = x_input[off+0]*w_k[1] + x_input[off+1]*w_k[5] + x_input[off+2]*w_k[9] + x_input[off+3]*w_k[13];
        x_k[base+2] = x_input[off+0]*w_k[2] + x_input[off+1]*w_k[6] + x_input[off+2]*w_k[10]+ x_input[off+3]*w_k[14];
        x_k[base+3] = x_input[off+0]*w_k[3] + x_input[off+1]*w_k[7] + x_input[off+2]*w_k[11]+ x_input[off+3]*w_k[15];

        // V1
        x_v[base+0] = x_input[off+0]*w_v[0] + x_input[off+1]*w_v[4] + x_input[off+2]*w_v[8] + x_input[off+3]*w_v[12];
        x_v[base+1] = x_input[off+0]*w_v[1] + x_input[off+1]*w_v[5] + x_input[off+2]*w_v[9] + x_input[off+3]*w_v[13];
        x_v[base+2] = x_input[off+0]*w_v[2] + x_input[off+1]*w_v[6] + x_input[off+2]*w_v[10]+ x_input[off+3]*w_v[14];
        x_v[base+3] = x_input[off+0]*w_v[3] + x_input[off+1]*w_v[7] + x_input[off+2]*w_v[11]+ x_input[off+3]*w_v[15];
    }

    // -------------------------------------------------------------
    // 2. Scaled dot‑product attention (manual unrolling, fused ops)
    // -------------------------------------------------------------
    // attention scores matrix (seq_len x seq_len)
    std::vector<float> x_score(seq_len * seq_len);
    // score_00
    x_score[0] = (x_q[0]*x_k[0] + x_q[1]*x_k[1] + x_q[2]*x_k[2] + x_q[3]*x_k[3]) * scale;
    // score_01
    x_score[1] = (x_q[0]*x_k[4] + x_q[1]*x_k[5] + x_q[2]*x_k[6] + x_q[3]*x_k[7]) * scale;
    // score_10
    x_score[2] = (x_q[4]*x_k[0] + x_q[5]*x_k[1] + x_q[6]*x_k[2] + x_q[7]*x_k[3]) * scale;
    // score_11
    x_score[3] = (x_q[4]*x_k[4] + x_q[5]*x_k[5] + x_q[6]*x_k[6] + x_q[7]*x_k[7]) * scale;

    // Softmax per row (fused exp & division)
    std::vector<float> x_attn(seq_len * seq_len);
    // row 0
    float e00 = std::exp(x_score[0]);
    float e01 = std::exp(x_score[1]);
    float sum0 = e00 + e01;
    x_attn[0] = e00 / sum0;
    x_attn[1] = e01 / sum0;
    // row 1
    float e10 = std::exp(x_score[2]);
    float e11 = std::exp(x_score[3]);
    float sum1 = e10 + e11;
    x_attn[2] = e10 / sum1;
    x_attn[3] = e11 / sum1;

    // -------------------------------------------------------------
    // 3. Context = Σ softmax * V   (manual unroll)
    // -------------------------------------------------------------
    std::vector<float> x_ctx(seq_len * hidden_sz);
    // token 0 context
    {
        int out = 0;
        // weighted sum of V0 and V1
        x_ctx[out+0] = x_attn[0]*x_v[0] + x_attn[1]*x_v[4];
        x_ctx[out+1] = x_attn[0]*x_v[1] + x_attn[1]*x_v[5];
        x_ctx[out+2] = x_attn[0]*x_v[2] + x_attn[1]*x_v[6];
        x_ctx[out+3] = x_attn[0]*x_v[3] + x_attn[1]*x_v[7];
    }
    // token 1 context
    {
        int out = hidden_sz;
        x_ctx[out+0] = x_attn[2]*x_v[0] + x_attn[3]*x_v[4];
        x_ctx[out+1] = x_attn[2]*x_v[1] + x_attn[3]*x_v[5];
        x_ctx[out+2] = x_attn[2]*x_v[2] + x_attn[3]*x_v[6];
        x_ctx[out+3] = x_attn[2]*x_v[3] + x_attn[3]*x_v[7];
    }

    // -------------------------------------------------------------
    // 4. Output projection + residual (still inside main)
    // -------------------------------------------------------------
    std::vector<float> x_proj(seq_len * hidden_sz);
    // token 0 projection
    {
        int in = 0, out = 0;
        x_proj[out+0] = x_ctx[in+0]*w_o[0] + x_ctx[in+1]*w_o[4] + x_ctx[in+2]*w_o[8] + x_ctx[in+3]*w_o[12];
        x_proj[out+1] = x_ctx[in+0]*w_o[1] + x_ctx[in+1]*w_o[5] + x_ctx[in+2]*w_o[9] + x_ctx[in+3]*w_o[13];
        x_proj[out+2] = x_ctx[in+0]*w_o[2] + x_ctx[in+1]*w_o[6] + x_ctx[in+2]*w_o[10]+ x_ctx[in+3]*w_o[14];
        x_proj[out+3] = x_ctx[in+0]*w_o[3] + x_ctx[in+1]*w_o[7] + x_ctx[in+2]*w_o[11]+ x_ctx[in+3]*w_o[15];
    }
    // token 1 projection
    {
        int in = hidden_sz, out = hidden_sz;
        x_proj[out+0] = x_ctx[in+0]*w_o[0] + x_ctx[in+1]*w_o[4] + x_ctx[in+2]*w_o[8] + x_ctx[in+3]*w_o[12];
        x_proj[out+1] = x_ctx[in+0]*w_o[1] + x_ctx[in+1]*w_o[5] + x_ctx[in+2]*w_o[9] + x_ctx[in+3]*w_o[13];
        x_proj[out+2] = x_ctx[in+0]*w_o[2] + x_ctx[in+1]*w_o[6] + x_ctx[in+2]*w_o[10]+ x_ctx[in+3]*w_o[14];
        x_proj[out+3] = x_ctx[in+0]*w_o[3] + x_ctx[in+1]*w_o[7] + x_ctx[in+2]*w_o[11]+ x_ctx[in+3]*w_o[15];
    }

    // Residual addition (attention output + original input)
    std::vector<float> x_res1(seq_len * hidden_sz);
    for (int i = 0; i < seq_len * hidden_sz; ++i) {
        x_res1[i] = x_proj[i] + x_input[i];
    }

    // -------------------------------------------------------------
    // 5. Feed‑forward network (hidden -> inter -> hidden) with ReLU
    // -------------------------------------------------------------
    std::vector<float> x_ff1(seq_len * inter_sz);
    // Linear 1 (manual unroll for each token)
    for (int token = 0; token < seq_len; ++token) {
        int in_off = token * hidden_sz;
        int out_off = token * inter_sz;
        // each of the 8 intermediate units
        for (int j = 0; j < inter_sz; ++j) {
            // fused sum of 4 products
            x_ff1[out_off + j] =
                x_res1[in_off + 0] * w_1[0 * inter_sz + j] +
                x_res1[in_off + 1] * w_1[1 * inter_sz + j] +
                x_res1[in_off + 2] * w_1[2 * inter_sz + j] +
                x_res1[in_off + 3] * w_1[3 * inter_sz + j];
            // ReLU activation (fused)
            if (x_ff1[out_off + j] < 0.0f) x_ff1[out_off + j] = 0.0f;
        }
    }

    // Linear 2 back to hidden
    std::vector<float> x_ff2(seq_len * hidden_sz);
    for (int token = 0; token < seq_len; ++token) {
        int in_off = token * inter_sz;
        int out_off = token * hidden_sz;
        for (int j = 0; j < hidden_sz; ++j) {
            x_ff2[out_off + j] =
                x_ff1[in_off + 0] * w_2[0 * hidden_sz + j] +
                x_ff1[in_off + 1] * w_2[1 * hidden_sz + j] +
                x_ff1[in_off + 2] * w_2[2 * hidden_sz + j] +
                x_ff1[in_off + 3] * w_2[3 * hidden_sz + j] +
                x_ff1[in_off + 4] * w_2[4 * hidden_sz + j] +
                x_ff1[in_off + 5] * w_2[5 * hidden_sz + j] +
                x_ff1[in_off + 6] * w_2[6 * hidden_sz + j] +
                x_ff1[in_off + 7] * w_2[7 * hidden_sz + j];
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
    std::cout << "ALBERT‑like forward output (deterministic test vectors):\n";
    for (int t = 0; t < seq_len; ++t) {
        std::cout << "Token " << t << ": ";
        for (int h = 0; h < hidden_sz; ++h) {
            std::cout << x_final[t * hidden_sz + h] << ' ';
        }
        std::cout << '\n';
    }
    return 0;
}
