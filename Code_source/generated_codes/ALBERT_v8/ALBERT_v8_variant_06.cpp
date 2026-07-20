#include <iostream>
#include <cmath>

/* LLM input variant 6: ordered-structured */

int main() {
    // -------------------------------------------------
    // Tiny ALBERT forward pass (version #8)
    // All data lives on the stack, no dynamic allocation.
    // -------------------------------------------------

    // ------------------- Hyper‑parameters -------------------
    int vocab_size   = 8;   // tiny vocab
    int embed_dim    = 4;   // hidden size
    int seq_len      = 4;   // sequence length (edge‑case: contains padding id 0)
    int ff_hidden    = 4;   // feed‑forward hidden size (same as embed_dim for ALBERT)

    // ------------------- Input (ordered‑structured) -------------------
    // Sorted token IDs including padding token (0)
    int input_ids[4] = {0, 1, 2, 3};

    // ------------------- Embedding matrix (structured) -------------------
    // token_emb[i][j] = 0.1 * (i+1) * (j+1)
    float token_emb[8][4] = {
        {0.1f*1*1, 0.1f*1*2, 0.1f*1*3, 0.1f*1*4},
        {0.1f*2*1, 0.1f*2*2, 0.1f*2*3, 0.1f*2*4},
        {0.1f*3*1, 0.1f*3*2, 0.1f*3*3, 0.1f*3*4},
        {0.1f*4*1, 0.1f*4*2, 0.1f*4*3, 0.1f*4*4},
        {0.1f*5*1, 0.1f*5*2, 0.1f*5*3, 0.1f*5*4},
        {0.1f*6*1, 0.1f*6*2, 0.1f*6*3, 0.1f*6*4},
        {0.1f*7*1, 0.1f*7*2, 0.1f*7*3, 0.1f*7*4},
        {0.1f*8*1, 0.1f*8*2, 0.1f*8*3, 0.1f*8*4}
    };

    // ------------------- Linear projection weights (shared, identity‑scaled) -------------------
    float proj_w[4][4] = {
        {0.1f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.1f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.1f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.1f}
    };
    float proj_b[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    // ------------------- Feed‑forward weights (identity‑scaled) -------------------
    float ff1_w[4][4] = {
        {0.1f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.1f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.1f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.1f}
    };
    float ff1_b[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    float ff2_w[4][4] = {
        {0.1f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.1f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.1f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.1f}
    };
    float ff2_b[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    // ------------------- Embedding lookup -------------------
    float embed[4][4];
    int i = 0;
    while (i < seq_len) {
        int id = input_ids[i];
        embed[i][0] = token_emb[id][0];
        embed[i][1] = token_emb[id][1];
        embed[i][2] = token_emb[id][2];
        embed[i][3] = token_emb[id][3];
        i = i + 1;
    }

    // ------------------- Compute Q = K = V (shared projection) -------------------
    float qkv[4][4];
    i = 0;
    while (i < seq_len) {
        float a0 = proj_w[0][0] * embed[i][0] + proj_w[0][1] * embed[i][1] +
                   proj_w[0][2] * embed[i][2] + proj_w[0][3] * embed[i][3] + proj_b[0];
        float a1 = proj_w[1][0] * embed[i][0] + proj_w[1][1] * embed[i][1] +
                   proj_w[1][2] * embed[i][2] + proj_w[1][3] * embed[i][3] + proj_b[1];
        float a2 = proj_w[2][0] * embed[i][0] + proj_w[2][1] * embed[i][1] +
                   proj_w[2][2] * embed[i][2] + proj_w[2][3] * embed[i][3] + proj_b[2];
        float a3 = proj_w[3][0] * embed[i][0] + proj_w[3][1] * embed[i][1] +
                   proj_w[3][2] * embed[i][2] + proj_w[3][3] * embed[i][3] + proj_b[3];
        qkv[i][0] = a0;
        qkv[i][1] = a1;
        qkv[i][2] = a2;
        qkv[i][3] = a3;
        i = i + 1;
    }

    // ------------------- Scaled dot‑product attention -------------------
    float scores[4][4];
    i = 0;
    while (i < seq_len) {
        int j = 0;
        while (j < seq_len) {
            float dot = qkv[i][0] * qkv[j][0] + qkv[i][1] * qkv[j][1] +
                        qkv[i][2] * qkv[j][2] + qkv[i][3] * qkv[j][3];
            scores[i][j] = dot / sqrtf((float)embed_dim);
            j = j + 1;
        }
        i = i + 1;
    }

    // ------------------- Softmax (row‑wise) -------------------
    float attn[4][4];
    i = 0;
    while (i < seq_len) {
        float mx = scores[i][0];
        int k = 1;
        while (k < seq_len) {
            if (scores[i][k] > mx) mx = scores[i][k];
            k = k + 1;
        }
        float sum = 0.0f;
        k = 0;
        while (k < seq_len) {
            float e = expf(scores[i][k] - mx);
            attn[i][k] = e;
            sum = sum + e;
            k = k + 1;
        }
        k = 0;
        while (k < seq_len) {
            attn[i][k] = attn[i][k] / sum;
            k = k + 1;
        }
        i = i + 1;
    }

    // ------------------- Context = Σ attn * V -------------------
    float context[4][4];
    i = 0;
    while (i < seq_len) {
        context[i][0] = 0.0f; context[i][1] = 0.0f;
        context[i][2] = 0.0f; context[i][3] = 0.0f;
        int j = 0;
        while (j < seq_len) {
            float coeff = attn[i][j];
            context[i][0] = context[i][0] + coeff * qkv[j][0];
            context[i][1] = context[i][1] + coeff * qkv[j][1];
            context[i][2] = context[i][2] + coeff * qkv[j][2];
            context[i][3] = context[i][3] + coeff * qkv[j][3];
            j = j + 1;
        }
        i = i + 1;
    }

    // ------------------- Feed‑forward network (GELU approximation) -------------------
    float hidden[4][4];
    i = 0;
    while (i < seq_len) {
        float l0 = ff1_w[0][0] * context[i][0] + ff1_w[0][1] * context[i][1] +
                   ff1_w[0][2] * context[i][2] + ff1_w[0][3] * context[i][3] + ff1_b[0];
        float l1 = ff1_w[1][0] * context[i][0] + ff1_w[1][1] * context[i][1] +
                   ff1_w[1][2] * context[i][2] + ff1_w[1][3] * context[i][3] + ff1_b[1];
        float l2 = ff1_w[2][0] * context[i][0] + ff1_w[2][1] * context[i][1] +
                   ff1_w[2][2] * context[i][2] + ff1_w[2][3] * context[i][3] + ff1_b[2];
        float l3 = ff1_w[3][0] * context[i][0] + ff1_w[3][1] * context[i][1] +
                   ff1_w[3][2] * context[i][2] + ff1_w[3][3] * context[i][3] + ff1_b[3];

        float g0 = 0.5f * l0 *
                   (1.0f + tanhf(0.7978845608f * (l0 + 0.044715f * l0 * l0 * l0)));
        float g1 = 0.5f * l1 *
                   (1.0f + tanhf(0.7978845608f * (l1 + 0.044715f * l1 * l1 * l1)));
        float g2 = 0.5f * l2 *
                   (1.0f + tanhf(0.7978845608f * (l2 + 0.044715f * l2 * l2 * l2)));
        float g3 = 0.5f * l3 *
                   (1.0f + tanhf(0.7978845608f * (l3 + 0.044715f * l3 * l3 * l3)));

        float o0 = ff2_w[0][0] * g0 + ff2_w[0][1] * g1 +
                   ff2_w[0][2] * g2 + ff2_w[0][3] * g3 + ff2_b[0];
        float o1 = ff2_w[1][0] * g0 + ff2_w[1][1] * g1 +
                   ff2_w[1][2] * g2 + ff2_w[1][3] * g3 + ff2_b[1];
        float o2 = ff2_w[2][0] * g0 + ff2_w[2][1] * g1 +
                   ff2_w[2][2] * g2 + ff2_w[2][3] * g3 + ff2_b[2];
        float o3 = ff2_w[3][0] * g0 + ff2_w[3][1] * g1 +
                   ff2_w[3][2] * g2 + ff2_w[3][3] * g3 + ff2_b[3];

        hidden[i][0] = o0;
        hidden[i][1] = o1;
        hidden[i][2] = o2;
        hidden[i][3] = o3;

        i = i + 1;
    }

    // ------------------- Print final hidden states -------------------
    std::cout << "ALBERT tiny forward output (seq_len=" << seq_len << "):\n";
    i = 0;
    while (i < seq_len) {
        std::cout << "Token " << i << " (id " << input_ids[i] << "): ";
        std::cout << hidden[i][0] << " " << hidden[i][1] << " "
                  << hidden[i][2] << " " << hidden[i][3] << "\n";
        i = i + 1;
    }

    return 0;
}
