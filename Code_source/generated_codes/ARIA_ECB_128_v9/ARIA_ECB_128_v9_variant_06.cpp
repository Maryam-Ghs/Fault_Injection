#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 6: ordered-structured */
int main() {
    /* -------------------- 1. Prepare S‑boxes -------------------- */
    std::vector<int> sboxA(256);
    std::vector<int> sboxB(256);
    for (int i = 0; i < 256; ++i) {
        int x = i;
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

    /* -------------------- 3. Key and plaintext -------------------- */
    std::vector<int> keyBytes = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F
    };
    std::vector<int> plainBytes = {
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F
    };

    /* -------------------- 4. Derive round keys -------------------- */
    std::vector< std::vector<int> > roundKey(13, std::vector<int>(16));
    roundKey[0] = keyBytes;
    for (int r = 1; r <= 12; ++r) {
        for (int i = 0; i < 16; ++i)
            roundKey[r][i] = roundKey[r-1][i] ^ rcon[r-1][i];
    }

    /* -------------------- 5. Helper lambdas -------------------- */
    auto subBytes = [&](std::vector<int>& st, int rnd) {
        int sel = rnd & 1;
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

    /* -------------------- 6. Encryption (ECB, single block) -------------------- */
    std::vector<int> state = plainBytes;

    for (int r = 0; r < 12; ++r) {
        addRoundKey(state, roundKey[r]);
        subBytes(state, r);
        linearMix(state);
    }
    addRoundKey(state, roundKey[12]);

    /* -------------------- 7. Output -------------------- */
    std::cout << "Plaintext : ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << state[i];
    std::cout << std::dec << "\n";

    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << state[i];
    std::cout << std::dec << "\n";

    return 0;
}
