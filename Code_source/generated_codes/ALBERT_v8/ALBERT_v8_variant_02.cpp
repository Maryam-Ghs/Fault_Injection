#include <iostream>
#include <cmath>

/* LLM input variant 2: small-diverse */

int main() {
    // -------------------------------------------------
    // Tiny ALBERT forward pass (version #8)
    // All data lives on the stack, no dynamic allocation.
    // -------------------------------------------------

    // ------------------- Hyper‑parameters -------------------
    int vocab_size   = 10;  // tiny vocab (expanded)
    int embed_dim    = 5;   // hidden size (expanded)
    int seq_len      = 5;   // sequence length (small, diverse)
    int ff_hidden    = 5;   // feed‑forward hidden size (matches embed_dim)

    // ------------------- Input (diverse) -------------------
    // Token IDs: includes a padding token (0) and repeated tokens (7)
    int input_ids[5] = {2, 0, 7, 7, 4};

    // ------------------- Embedding matrix -------------------
    float token_emb[10][5] = {
        {0.05f, 0.10f, 0.15f, 0.20f, 0.25f},   // id 0 : padding (small values)
        {0.30f, 0.35f, 0.40f, 0.45f, 0.50f},
        {0.55f, 0.60f, 0.65f, 0.70f, 0.75f},
        {0.80f, 0.85f, 0.90f, 0.95f, 1.00f},
        {1.05f, 1.10f, 1.15f, 1.20f, 1.25f},
        {1.30f, 1.35f, 1.40f, 1.45f, 1.50f},
        {1.55f, 1.60f, 1.65f, 1.70f, 1.75f},
        {1.80f, 1.85f, 1.90f, 1.95f, 2.00f},
        {2.05f, 2.10f, 2.15f, 2.20f, 2.25f},
        {2.30f, 2.35f, 2.40f, 2.45f, 2.50f}
    };

    // ------------------- Linear projection weights (shared) -------------------
    float proj_w[5][5] = {
        {0.01f, 0.02f, 0.03f, 0.04f, 0.05f},
        {0.06f, 0.07f, 0.08f, 0.09f, 0.10f},
        {0.11f, 0.12f, 0.13f, 0.14f, 0.15f},
        {0.16f, 0.17f, 0.18f, 0.19f, 0.20f},
        {0.21f, 0.22f, 0.23f, 0.24f, 0.25f}
    };
    float proj_b[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    // ------------------- Feed‑forward weights -------------------
    float ff1_w[5][5] = {
        {0.01f, 0.02f, 0.03f, 0.04f, 0.05f},
        {0.06f, 0.07f, 0.08f, 0.09f, 0.10f},
        {0.11f, 0.12f, 0.13f, 0.14f, 0.15f},
        {0.16f, 0.17f, 0.18f, 0.19f, 0.20f},
        {0.21f, 0.22f, 0.23f, 0.24f, 0.25f}
    };
    float ff1_b[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    float ff2_w[5][5] = {
        {0.01f, 0.02f, 0.03f, 0.04f, 0.05f},
        {0.06f, 0.07f, 0.08f, 0.09f, 0.10f},
        {0.11f, 0.12f, 0.13f, 0.14f, 0.15f},
        {0.16f, 0.17f, 0.18f, 0.19f, 0.20f},
        {0.21f, 0.22f, 0.23f, 0.24f, 0.25f}
    };
    float ff2_b[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    // ------------------- Embedding lookup -------------------
    float embed[5][5];
    int i = 0;
    while (i < seq_len) {
        int id = input_ids[i];
        embed[i][0] = token_emb[id][0];
        embed[i][1] = token_emb[id][1];
        embed[i][2] = token_emb[id][2];
        embed[i][3] = token_emb[id][3];
        embed[i][4] = token_emb[id][4];
        i = i + 1;
    }

    // ------------------- Compute Q = K = V (shared projection) -------------------
    float qkv[5][5];
    i = 0;
    while (i < seq_len) {
        float a0 = proj_w[0][0] * embed[i][0] + proj_w[0][1] * embed[i][1] +
                   proj_w[0][2] * embed[i][2] + proj_w[0][3] * embed[i][3] +
                   proj_w[0][4] * embed[i][4] + proj_b[0];
        float a1 = proj_w[1][0] * embed[i][0] + proj_w[1][1] * embed[i][1] +
                   proj_w[1][2] * embed[i][2] + proj_w[1][3] * embed[i][3] +
                   proj_w[1][4] * embed[i][4] + proj_b[1];
        float a2 = proj_w[2][0] * embed[i][0] + proj_w[2][1] * embed[i][1] +
                   proj_w[2][2] * embed[i][2] + proj_w[2][3] * embed[i][3] +
                   proj_w[2][4] * embed[i][4] + proj_b[2];
        float a3 = proj_w[3][0] * embed[i][0] + proj_w[3][1] * embed[i][1] +
                   proj_w[3][2] * embed[i][2] + proj_w[3][3] * embed[i][3] +
                   proj_w[3][4] * embed[i][4] + proj_b[3];
        float a4 = proj_w[4][0] * embed[i][0] + proj_w[4][1] * embed[i][1] +
                   proj_w[4][2] * embed[i][2] + proj_w[4][3] * embed[i][3] +
                   proj_w[4][4] * embed[i][4] + proj_b[4];
        qkv[i][0] = a0;
        qkv[i][1] = a1;
        qkv[i][2] = a2;
        qkv[i][3] = a3;
        qkv[i][4] = a4;
        i = i + 1;
    }

    // ------------------- Scaled dot‑product attention -------------------
    float scores[5][5];
    i = 0;
    while (i < seq_len) {
        int j = 0;
        while (j < seq_len) {
            float dot = qkv[i][0] * qkv[j][0] + qkv[i][1] * qkv[j][1] +
                        qkv[i][2] * qkv[j][2] + qkv[i][3] * qkv[j][3] +
                        qkv[i][4] * qkv[j][4];
            scores[i][j] = dot / sqrtf((float)embed_dim);
            j = j + 1;
        }
        i = i + 1;
    }

    // ------------------- Softmax (row‑wise) -------------------
    float attn[5][5];
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
    float context[5][5];
    i = 0;
    while (i < seq_len) {
        context[i][0] = 0.0f; context[i][1] = 0.0f;
        context[i][2] = 0.0f; context[i][3] = 0.0f;
        context[i][4] = 0.0f;
        int j = 0;
        while (j < seq_len) {
            float coeff = attn[i][j];
            context[i][0] = context[i][0] + coeff * qkv[j][0];
            context[i][1] = context[i][1] + coeff * qkv[j][1];
            context[i][2] = context[i][2] + coeff * qkv[j][2];
            context[i][3] = context[i][3] + coeff * qkv[j][3];
            context[i][4] = context[i][4] + coeff * qkv[j][4];
            j = j + 1;
        }
        i = i + 1;
    }

    // ------------------- Feed‑forward network (GELU approximation) -------------------
    float hidden[5][5];
    i = 0;
    while (i < seq_len) {
        float l0 = ff1_w[0][0] * context[i][0] + ff1_w[0][1] * context[i][1] +
                   ff1_w[0][2] * context[i][2] + ff1_w[0][3] * context[i][3] +
                   ff1_w[0][4] * context[i][4] + ff1_b[0];
        float l1 = ff1_w[1][0] * context[i][0] + ff1_w[1][1] * context[i][1] +
                   ff1_w[1][2] * context[i][2] + ff1_w[1][3] * context[i][3] +
                   ff1_w[1][4] * context[i][4] + ff1_b[1];
        float l2 = ff1_w[2][0] * context[i][0] + ff1_w[2][1] * context[i][1] +
                   ff1_w[2][2] * context[i][2] + ff1_w[2][3] * context[i][3] +
                   ff1_w[2][4] * context[i][4] + ff1_b[2];
        float l3 = ff1_w[3][0] * context[i][0] + ff1_w[3][1] * context[i][1] +
                   ff1_w[3][2] * context[i][2] + ff1_w[3][3] * context[i][3] +
                   ff1_w[3][4] * context[i][4] + ff1_b[3];
        float l4 = ff1_w[4][0] * context[i][0] + ff1_w[4][1] * context[i][1] +
                   ff1_w[4][2] * context[i][2] + ff1_w[4][3] * context[i][3] +
                   ff1_w[4][4] * context[i][4] + ff1_b[4];

        float g0 = 0.5f * l0 *
                   (1.0f + tanhf(0.7978845608f * (l0 + 0.044715f * l0 * l0 * l0)));
        float g1 = 0.5f * l1 *
                   (1.0f + tanhf(0.7978845608f * (l1 + 0.044715f * l1 * l1 * l1)));
        float g2 = 0.5f * l2 *
                   (1.0f + tanhf(0.7978845608f * (l2 + 0.044715f * l2 * l2 * l2)));
        float g3 = 0.5f * l3 *
                   (1.0f + tanhf(0.7978845608f * (l3 + 0.044715f * l3 * l3 * l3)));
        float g4 = 0.5f * l4 *
                   (1.0f + tanhf(0.7978845608f * (l4 + 0.044715f * l4 * l4 * l4)));

        float o0 = ff2_w[0][0] * g0 + ff2_w[0][1] * g1 + ff2_w[0][2] * g2 +
                   ff2_w[0][3] * g3 + ff2_w[0][4] * g4 + ff2_b[0];
        float o1 = ff2_w[1][0] * g0 + ff2_w[1][1] * g1 + ff2_w[1][2] * g2 +
                   ff2_w[1][3] * g3 + ff2_w[1][4] * g4 + ff2_b[1];
        float o2 = ff2_w[2][0] * g0 + ff2_w[2][1] * g1 + ff2_w[2][2] * g2 +
                   ff2_w[2][3] * g3 + ff2_w[2][4] * g4 + ff2_b[2];
        float o3 = ff2_w[3][0] * g0 + ff2_w[3][1] * g1 + ff2_w[3][2] * g2 +
                   ff2_w[3][3] * g3 + ff2_w[3][4] * g4 + ff2_b[3];
        float o4 = ff2_w[4][0] * g0 + ff2_w[4][1] * g1 + ff2_w[4][2] * g2 +
                   ff2_w[4][3] * g3 + ff2_w[4][4] * g4 + ff2_b[4];

        hidden[i][0] = o0;
        hidden[i][1] = o1;
        hidden[i][2] = o2;
        hidden[i][3] = o3;
        hidden[i][4] = o4;

        i = i + 1;
    }

    // ------------------- Print final hidden states -------------------
    std::cout << "ALBERT tiny forward output (seq_len=" << seq_len << "):\n";
    i = 0;
    while (i < seq_len) {
        std::cout << "Token " << i << " (id " << input_ids[i] << "): ";
        std::cout << hidden[i][0] << " " << hidden[i][1] << " "
                  << hidden[i][2] << " " << hidden[i][3] << " "
                  << hidden[i][4] << "\n";
        i = i + 1;
    }

    return 0;
}
