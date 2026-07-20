/* LLM input variant 10: large-safe-stress */
#include <iostream>
#include <cmath>

int main() {
    // -------------------------------------------------
    // Tiny ALBERT forward pass (version #8)
    // All data lives on the stack, no dynamic allocation.
    // -------------------------------------------------

    // ------------------- Hyper‑parameters -------------------
    const int vocab_size = 16;   // larger vocab
    const int embed_dim  = 8;    // larger hidden size
    const int seq_len    = 16;   // longer sequence (includes padding id 0)
    const int ff_hidden  = 8;    // feed‑forward hidden size (same as embed_dim for ALBERT)

    // ------------------- Input (edge‑case heavy) -------------------
    // Token IDs: includes a padding token (0) and several repeated tokens
    int input_ids[16] = {1, 0, 5, 5, 8, 8, 12, 12, 15, 15, 2, 3, 4, 6, 7, 9};

    // ------------------- Embedding matrix -------------------
    float token_emb[16][8] = {
        {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},   // id 0 : padding
        {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f},
        {0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f},
        {1.7f, 1.8f, 1.9f, 2.0f, 2.1f, 2.2f, 2.3f, 2.4f},
        {2.5f, 2.6f, 2.7f, 2.8f, 2.9f, 3.0f, 3.1f, 3.2f},
        {3.3f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f, 3.9f, 4.0f},
        {4.1f, 4.2f, 4.3f, 4.4f, 4.5f, 4.6f, 4.7f, 4.8f},
        {4.9f, 5.0f, 5.1f, 5.2f, 5.3f, 5.4f, 5.5f, 5.6f},
        {5.7f, 5.8f, 5.9f, 6.0f, 6.1f, 6.2f, 6.3f, 6.4f},
        {6.5f, 6.6f, 6.7f, 6.8f, 6.9f, 7.0f, 7.1f, 7.2f},
        {7.3f, 7.4f, 7.5f, 7.6f, 7.7f, 7.8f, 7.9f, 8.0f},
        {8.1f, 8.2f, 8.3f, 8.4f, 8.5f, 8.6f, 8.7f, 8.8f},
        {8.9f, 9.0f, 9.1f, 9.2f, 9.3f, 9.4f, 9.5f, 9.6f},
        {9.7f, 9.8f, 9.9f,10.0f,10.1f,10.2f,10.3f,10.4f},
        {10.5f,10.6f,10.7f,10.8f,10.9f,11.0f,11.1f,11.2f},
        {11.3f,11.4f,11.5f,11.6f,11.7f,11.8f,11.9f,12.0f}
    };

    // ------------------- Linear projection weights (shared) -------------------
    float proj_w[8][8] = {
        {0.01f,0.02f,0.03f,0.04f,0.05f,0.06f,0.07f,0.08f},
        {0.09f,0.10f,0.11f,0.12f,0.13f,0.14f,0.15f,0.16f},
        {0.17f,0.18f,0.19f,0.20f,0.21f,0.22f,0.23f,0.24f},
        {0.25f,0.26f,0.27f,0.28f,0.29f,0.30f,0.31f,0.32f},
        {0.33f,0.34f,0.35f,0.36f,0.37f,0.38f,0.39f,0.40f},
        {0.41f,0.42f,0.43f,0.44f,0.45f,0.46f,0.47f,0.48f},
        {0.49f,0.50f,0.51f,0.52f,0.53f,0.54f,0.55f,0.56f},
        {0.57f,0.58f,0.59f,0.60f,0.61f,0.62f,0.63f,0.64f}
    };
    float proj_b[8] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};

    // ------------------- Feed‑forward weights -------------------
    float ff1_w[8][8] = {
        {0.01f,0.02f,0.03f,0.04f,0.05f,0.06f,0.07f,0.08f},
        {0.09f,0.10f,0.11f,0.12f,0.13f,0.14f,0.15f,0.16f},
        {0.17f,0.18f,0.19f,0.20f,0.21f,0.22f,0.23f,0.24f},
        {0.25f,0.26f,0.27f,0.28f,0.29f,0.30f,0.31f,0.32f},
        {0.33f,0.34f,0.35f,0.36f,0.37f,0.38f,0.39f,0.40f},
        {0.41f,0.42f,0.43f,0.44f,0.45f,0.46f,0.47f,0.48f},
        {0.49f,0.50f,0.51f,0.52f,0.53f,0.54f,0.55f,0.56f},
        {0.57f,0.58f,0.59f,0.60f,0.61f,0.62f,0.63f,0.64f}
    };
    float ff1_b[8] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};

    float ff2_w[8][8] = {
        {0.01f,0.02f,0.03f,0.04f,0.05f,0.06f,0.07f,0.08f},
        {0.09f,0.10f,0.11f,0.12f,0.13f,0.14f,0.15f,0.16f},
        {0.17f,0.18f,0.19f,0.20f,0.21f,0.22f,0.23f,0.24f},
        {0.25f,0.26f,0.27f,0.28f,0.29f,0.30f,0.31f,0.32f},
        {0.33f,0.34f,0.35f,0.36f,0.37f,0.38f,0.39f,0.40f},
        {0.41f,0.42f,0.43f,0.44f,0.45f,0.46f,0.47f,0.48f},
        {0.49f,0.50f,0.51f,0.52f,0.53f,0.54f,0.55f,0.56f},
        {0.57f,0.58f,0.59f,0.60f,0.61f,0.62f,0.63f,0.64f}
    };
    float ff2_b[8] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};

    // ------------------- Embedding lookup -------------------
    float embed[16][8];
    int i = 0;
    while (i < seq_len) {
        int id = input_ids[i];
        embed[i][0] = token_emb[id][0];
        embed[i][1] = token_emb[id][1];
        embed[i][2] = token_emb[id][2];
        embed[i][3] = token_emb[id][3];
        embed[i][4] = token_emb[id][4];
        embed[i][5] = token_emb[id][5];
        embed[i][6] = token_emb[id][6];
        embed[i][7] = token_emb[id][7];
        i = i + 1;
    }

    // ------------------- Compute Q = K = V (shared projection) -------------------
    float qkv[16][8];
    i = 0;
    while (i < seq_len) {
        float a0 = proj_w[0][0]*embed[i][0] + proj_w[0][1]*embed[i][1] + proj_w[0][2]*embed[i][2] + proj_w[0][3]*embed[i][3] +
                   proj_w[0][4]*embed[i][4] + proj_w[0][5]*embed[i][5] + proj_w[0][6]*embed[i][6] + proj_w[0][7]*embed[i][7] + proj_b[0];
        float a1 = proj_w[1][0]*embed[i][0] + proj_w[1][1]*embed[i][1] + proj_w[1][2]*embed[i][2] + proj_w[1][3]*embed[i][3] +
                   proj_w[1][4]*embed[i][4] + proj_w[1][5]*embed[i][5] + proj_w[1][6]*embed[i][6] + proj_w[1][7]*embed[i][7] + proj_b[1];
        float a2 = proj_w[2][0]*embed[i][0] + proj_w[2][1]*embed[i][1] + proj_w[2][2]*embed[i][2] + proj_w[2][3]*embed[i][3] +
                   proj_w[2][4]*embed[i][4] + proj_w[2][5]*embed[i][5] + proj_w[2][6]*embed[i][6] + proj_w[2][7]*embed[i][7] + proj_b[2];
        float a3 = proj_w[3][0]*embed[i][0] + proj_w[3][1]*embed[i][1] + proj_w[3][2]*embed[i][2] + proj_w[3][3]*embed[i][3] +
                   proj_w[3][4]*embed[i][4] + proj_w[3][5]*embed[i][5] + proj_w[3][6]*embed[i][6] + proj_w[3][7]*embed[i][7] + proj_b[3];
        float a4 = proj_w[4][0]*embed[i][0] + proj_w[4][1]*embed[i][1] + proj_w[4][2]*embed[i][2] + proj_w[4][3]*embed[i][3] +
                   proj_w[4][4]*embed[i][4] + proj_w[4][5]*embed[i][5] + proj_w[4][6]*embed[i][6] + proj_w[4][7]*embed[i][7] + proj_b[4];
        float a5 = proj_w[5][0]*embed[i][0] + proj_w[5][1]*embed[i][1] + proj_w[5][2]*embed[i][2] + proj_w[5][3]*embed[i][3] +
                   proj_w[5][4]*embed[i][4] + proj_w[5][5]*embed[i][5] + proj_w[5][6]*embed[i][6] + proj_w[5][7]*embed[i][7] + proj_b[5];
        float a6 = proj_w[6][0]*embed[i][0] + proj_w[6][1]*embed[i][1] + proj_w[6][2]*embed[i][2] + proj_w[6][3]*embed[i][3] +
                   proj_w[6][4]*embed[i][4] + proj_w[6][5]*embed[i][5] + proj_w[6][6]*embed[i][6] + proj_w[6][7]*embed[i][7] + proj_b[6];
        float a7 = proj_w[7][0]*embed[i][0] + proj_w[7][1]*embed[i][1] + proj_w[7][2]*embed[i][2] + proj_w[7][3]*embed[i][3] +
                   proj_w[7][4]*embed[i][4] + proj_w[7][5]*embed[i][5] + proj_w[7][6]*embed[i][6] + proj_w[7][7]*embed[i][7] + proj_b[7];
        qkv[i][0] = a0; qkv[i][1] = a1; qkv[i][2] = a2; qkv[i][3] = a3;
        qkv[i][4] = a4; qkv[i][5] = a5; qkv[i][6] = a6; qkv[i][7] = a7;
        i = i + 1;
    }

    // ------------------- Scaled dot‑product attention -------------------
    float scores[16][16];
    i = 0;
    while (i < seq_len) {
        int j = 0;
        while (j < seq_len) {
            float dot = qkv[i][0]*qkv[j][0] + qkv[i][1]*qkv[j][1] + qkv[i][2]*qkv[j][2] + qkv[i][3]*qkv[j][3] +
                        qkv[i][4]*qkv[j][4] + qkv[i][5]*qkv[j][5] + qkv[i][6]*qkv[j][6] + qkv[i][7]*qkv[j][7];
            scores[i][j] = dot / sqrtf((float)embed_dim);
            j = j + 1;
        }
        i = i + 1;
    }

    // ------------------- Softmax (row‑wise) -------------------
    float attn[16][16];
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
    float context[16][8];
    i = 0;
    while (i < seq_len) {
        context[i][0]=0.0f; context[i][1]=0.0f; context[i][2]=0.0f; context[i][3]=0.0f;
        context[i][4]=0.0f; context[i][5]=0.0f; context[i][6]=0.0f; context[i][7]=0.0f;
        int j = 0;
        while (j < seq_len) {
            float coeff = attn[i][j];
            context[i][0] += coeff * qkv[j][0];
            context[i][1] += coeff * qkv[j][1];
            context[i][2] += coeff * qkv[j][2];
            context[i][3] += coeff * qkv[j][3];
            context[i][4] += coeff * qkv[j][4];
            context[i][5] += coeff * qkv[j][5];
            context[i][6] += coeff * qkv[j][6];
            context[i][7] += coeff * qkv[j][7];
            j = j + 1;
        }
        i = i + 1;
    }

    // ------------------- Feed‑forward network (GELU approximation) -------------------
    float hidden[16][8];
    i = 0;
    while (i < seq_len) {
        // FF1 linear
        float l0 = ff1_w[0][0]*context[i][0] + ff1_w[0][1]*context[i][1] + ff1_w[0][2]*context[i][2] + ff1_w[0][3]*context[i][3] +
                   ff1_w[0][4]*context[i][4] + ff1_w[0][5]*context[i][5] + ff1_w[0][6]*context[i][6] + ff1_w[0][7]*context[i][7] + ff1_b[0];
        float l1 = ff1_w[1][0]*context[i][0] + ff1_w[1][1]*context[i][1] + ff1_w[1][2]*context[i][2] + ff1_w[1][3]*context[i][3] +
                   ff1_w[1][4]*context[i][4] + ff1_w[1][5]*context[i][5] + ff1_w[1][6]*context[i][6] + ff1_w[1][7]*context[i][7] + ff1_b[1];
        float l2 = ff1_w[2][0]*context[i][0] + ff1_w[2][1]*context[i][1] + ff1_w[2][2]*context[i][2] + ff1_w[2][3]*context[i][3] +
                   ff1_w[2][4]*context[i][4] + ff1_w[2][5]*context[i][5] + ff1_w[2][6]*context[i][6] + ff1_w[2][7]*context[i][7] + ff1_b[2];
        float l3 = ff1_w[3][0]*context[i][0] + ff1_w[3][1]*context[i][1] + ff1_w[3][2]*context[i][2] + ff1_w[3][3]*context[i][3] +
                   ff1_w[3][4]*context[i][4] + ff1_w[3][5]*context[i][5] + ff1_w[3][6]*context[i][6] + ff1_w[3][7]*context[i][7] + ff1_b[3];
        float l4 = ff1_w[4][0]*context[i][0] + ff1_w[4][1]*context[i][1] + ff1_w[4][2]*context[i][2] + ff1_w[4][3]*context[i][3] +
                   ff1_w[4][4]*context[i][4] + ff1_w[4][5]*context[i][5] + ff1_w[4][6]*context[i][6] + ff1_w[4][7]*context[i][7] + ff1_b[4];
        float l5 = ff1_w[5][0]*context[i][0] + ff1_w[5][1]*context[i][1] + ff1_w[5][2]*context[i][2] + ff1_w[5][3]*context[i][3] +
                   ff1_w[5][4]*context[i][4] + ff1_w[5][5]*context[i][5] + ff1_w[5][6]*context[i][6] + ff1_w[5][7]*context[i][7] + ff1_b[5];
        float l6 = ff1_w[6][0]*context[i][0] + ff1_w[6][1]*context[i][1] + ff1_w[6][2]*context[i][2] + ff1_w[6][3]*context[i][3] +
                   ff1_w[6][4]*context[i][4] + ff1_w[6][5]*context[i][5] + ff1_w[6][6]*context[i][6] + ff1_w[6][7]*context[i][7] + ff1_b[6];
        float l7 = ff1_w[7][0]*context[i][0] + ff1_w[7][1]*context[i][1] + ff1_w[7][2]*context[i][2] + ff1_w[7][3]*context[i][3] +
                   ff1_w[7][4]*context[i][4] + ff1_w[7][5]*context[i][5] + ff1_w[7][6]*context[i][6] + ff1_w[7][7]*context[i][7] + ff1_b[7];

        // GELU approx
        float g0 = 0.5f * l0 * (1.0f + tanhf(0.7978845608f * (l0 + 0.044715f * l0 * l0 * l0)));
        float g1 = 0.5f * l1 * (1.0f + tanhf(0.7978845608f * (l1 + 0.044715f * l1 * l1 * l1)));
        float g2 = 0.5f * l2 * (1.0f + tanhf(0.7978845608f * (l2 + 0.044715f * l2 * l2 * l2)));
        float g3 = 0.5f * l3 * (1.0f + tanhf(0.7978845608f * (l3 + 0.044715f * l3 * l3 * l3)));
        float g4 = 0.5f * l4 * (1.0f + tanhf(0.7978845608f * (l4 + 0.044715f * l4 * l4 * l4)));
        float g5 = 0.5f * l5 * (1.0f + tanhf(0.7978845608f * (l5 + 0.044715f * l5 * l5 * l5)));
        float g6 = 0.5f * l6 * (1.0f + tanhf(0.7978845608f * (l6 + 0.044715f * l6 * l6 * l6)));
        float g7 = 0.5f * l7 * (1.0f + tanhf(0.7978845608f * (l7 + 0.044715f * l7 * l7 * l7)));

        // FF2 linear
        float o0 = ff2_w[0][0]*g0 + ff2_w[0][1]*g1 + ff2_w[0][2]*g2 + ff2_w[0][3]*g3 +
                   ff2_w[0][4]*g4 + ff2_w[0][5]*g5 + ff2_w[0][6]*g6 + ff2_w[0][7]*g7 + ff2_b[0];
        float o1 = ff2_w[1][0]*g0 + ff2_w[1][1]*g1 + ff2_w[1][2]*g2 + ff2_w[1][3]*g3 +
                   ff2_w[1][4]*g4 + ff2_w[1][5]*g5 + ff2_w[1][6]*g6 + ff2_w[1][7]*g7 + ff2_b[1];
        float o2 = ff2_w[2][0]*g0 + ff2_w[2][1]*g1 + ff2_w[2][2]*g2 + ff2_w[2][3]*g3 +
                   ff2_w[2][4]*g4 + ff2_w[2][5]*g5 + ff2_w[2][6]*g6 + ff2_w[2][7]*g7 + ff2_b[2];
        float o3 = ff2_w[3][0]*g0 + ff2_w[3][1]*g1 + ff2_w[3][2]*g2 + ff2_w[3][3]*g3 +
                   ff2_w[3][4]*g4 + ff2_w[3][5]*g5 + ff2_w[3][6]*g6 + ff2_w[3][7]*g7 + ff2_b[3];
        float o4 = ff2_w[4][0]*g0 + ff2_w[4][1]*g1 + ff2_w[4][2]*g2 + ff2_w[4][3]*g3 +
                   ff2_w[4][4]*g4 + ff2_w[4][5]*g5 + ff2_w[4][6]*g6 + ff2_w[4][7]*g7 + ff2_b[4];
        float o5 = ff2_w[5][0]*g0 + ff2_w[5][1]*g1 + ff2_w[5][2]*g2 + ff2_w[5][3]*g3 +
                   ff2_w[5][4]*g4 + ff2_w[5][5]*g5 + ff2_w[5][6]*g6 + ff2_w[5][7]*g7 + ff2_b[5];
        float o6 = ff2_w[6][0]*g0 + ff2_w[6][1]*g1 + ff2_w[6][2]*g2 + ff2_w[6][3]*g3 +
                   ff2_w[6][4]*g4 + ff2_w[6][5]*g5 + ff2_w[6][6]*g6 + ff2_w[6][7]*g7 + ff2_b[6];
        float o7 = ff2_w[7][0]*g0 + ff2_w[7][1]*g1 + ff2_w[7][2]*g2 + ff2_w[7][3]*g3 +
                   ff2_w[7][4]*g4 + ff2_w[7][5]*g5 + ff2_w[7][6]*g6 + ff2_w[7][7]*g7 + ff2_b[7];

        hidden[i][0]=o0; hidden[i][1]=o1; hidden[i][2]=o2; hidden[i][3]=o3;
        hidden[i][4]=o4; hidden[i][5]=o5; hidden[i][6]=o6; hidden[i][7]=o7;

        i = i + 1;
    }

    // ------------------- Print final hidden states -------------------
    std::cout << "ALBERT tiny forward output (seq_len=" << seq_len << "):\n";
    i = 0;
    while (i < seq_len) {
        std::cout << "Token " << i << " (id " << input_ids[i] << "): ";
        std::cout << hidden[i][0] << " " << hidden[i][1] << " " << hidden[i][2] << " " << hidden[i][3] << " "
                  << hidden[i][4] << " " << hidden[i][5] << " " << hidden[i][6] << " " << hidden[i][7] << "\n";
        i = i + 1;
    }

    return 0;
}
