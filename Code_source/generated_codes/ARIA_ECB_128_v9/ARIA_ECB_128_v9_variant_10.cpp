#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

int main() {
    /* -------------------- 1. Prepare S‑boxes -------------------- */
    std::vector<int> sboxA(256);
    std::vector<int> sboxB(256);
    for (int i = 0; i < 256; ++i) {
        int x = i;
        /* a tiny handcrafted permutation – just for demonstration */
        x = ((x << 1) | (x >> 7)) & 0xFF;
        x ^= 0x63;
        sboxA[i] = x;
        sboxB[i] = ((x * 0x5B) ^ 0x35) & 0xFF;
    }

    /* -------------------- 2. Round constants -------------------- */
    std::vector< std::vector<int> > rcon(12, std::vector<int>(16));
    for (int r = 0; r < 12; ++r)
        for (int b = 0; b < 16; ++b)
            rcon[r][b] = ((r + 1) * (b + 1)) & 0xFF;

    /* -------------------- 3. Key (single master key) -------------------- */
    std::vector<int> keyBytes = {
        0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
        0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff
    };

    /* -------------------- 4. Generate a large plaintext dataset -------------------- */
    const int NUM_BLOCKS = 1024;               // safe large stress size
    std::vector<int> plainBytes;
    plainBytes.reserve(NUM_BLOCKS * 16);
    for (int i = 0; i < NUM_BLOCKS * 16; ++i) {
        // Deterministic pattern that stays within byte range
        plainBytes.push_back((i * 0x11) & 0xFF);
    }

    /* -------------------- 5. Derive round keys -------------------- */
    std::vector< std::vector<int> > roundKey(13, std::vector<int>(16));
    roundKey[0] = keyBytes;                     // K0 = master key
    for (int r = 1; r <= 12; ++r) {             // K1 … K12
        for (int i = 0; i < 16; ++i)
            roundKey[r][i] = roundKey[r-1][i] ^ rcon[r-1][i];
    }

    /* -------------------- 6. Helper lambdas -------------------- */
    auto subBytes = [&](std::vector<int>& st, int rnd) {
        int sel = rnd & 1;                       // 0 → A, 1 → B
        std::vector<int>& sb = (sel == 0) ? sboxA : sboxB;
        for (int i = 0; i < 16; ++i)
            st[i] = sb[st[i]];
    };

    auto linearMix = [&](std::vector<int>& st) {
        std::vector<int> tmp(16);
        for (int col = 0; col < 4; ++col) {
            int a0 = st[col];
            int a1 = st[4 + col];
            int a2 = st[8 + col];
            int a3 = st[12 + col];
            tmp[col]       = a0 ^ a1;
            tmp[4 + col]   = a1 ^ a2;
            tmp[8 + col]   = a2 ^ a3;
            tmp[12 + col]  = a3 ^ a0;
        }
        st.swap(tmp);
    };

    auto addRoundKey = [&](std::vector<int>& st, const std::vector<int>& rk) {
        for (int i = 0; i < 16; ++i)
            st[i] ^= rk[i];
    };

    /* -------------------- 7. Encrypt all blocks -------------------- */
    std::vector<int> ciphertext;
    ciphertext.reserve(NUM_BLOCKS * 16);

    for (int blk = 0; blk < NUM_BLOCKS; ++blk) {
        std::vector<int> state(plainBytes.begin() + blk * 16,
                              plainBytes.begin() + (blk + 1) * 16);

        for (int r = 0; r < 12; ++r) {
            addRoundKey(state, roundKey[r]);   // ⊕ Ki
            subBytes(state, r);                // Substitution
            linearMix(state);                  // Diffusion
        }
        addRoundKey(state, roundKey[12]);      // Final key addition

        ciphertext.insert(ciphertext.end(), state.begin(), state.end());
    }

    /* -------------------- 8. Output (first block as representative) -------------------- */
    std::cout << "Plaintext (first block) : ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << plainBytes[i];
    std::cout << std::dec << "\n";

    std::cout << "Ciphertext (first block) : ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << ciphertext[i];
    std::cout << std::dec << "\n";

    // Optionally, report total processed blocks to show stress level
    std::cout << "Total blocks encrypted: " << NUM_BLOCKS << "\n";

    return 0;
}
