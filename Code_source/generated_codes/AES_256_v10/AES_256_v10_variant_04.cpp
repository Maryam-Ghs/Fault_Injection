// AES-256 implementation – Version 10
// Uses only int (no unsigned, double, long, const)
// Stack‑allocated arrays, class‑based, branch‑light, fused expressions
/* LLM input variant 4: signed-extremes */

#include <iostream>
#include <iomanip>

class AES256 {
    // S‑box
    int sbox[256] = {
        99,124,119,123,242,107,111,197,48,1,103,43,254,215,171,118,
        202,130,201,125,250,89,71,240,173,212,162,175,156,164,114,192,
        183,253,147,38,54,63,247,204,52,165,229,241,113,216,49,21,
        4,199,35,195,24,150,5,154,7,18,128,226,235,39,178,117,
        9,131,44,26,27,110,90,160,82,59,214,179,41,227,47,132,
        83,209,0,237,32,252,177,91,106,203,190,57,74,76,88,207,
        208,239,170,251,67,77,51,133,69,249,2,127,80,60,159,168,
        81,163,64,143,146,157,56,245,188,182,218,33,16,255,243,210,
        205,12,19,236,95,151,68,23,196,167,126,61,100,93,25,115,
        96,129,79,220,34,42,144,136,70,238,184,20,222,94,11,219,
        224,50,58,10,73,6,36,92,194,211,172,98,145,149,228,121,
        231,200,55,109,141,213,78,169,108,86,244,234,101,122,174,8,
        186,120,37,46,28,166,180,198,232,221,116,31,75,189,139,138,
        112,62,181,102,72,3,246,14,97,53,87,185,134,193,29,158,
        225,248,152,17,105,217,142,148,155,30,135,233,206,85,40,223,
        140,161,137,13,191,230,66,104,65,153,45,15,176,84,187,22
    };

    // Rcon
    int rcon[15] = {1,2,4,8,16,32,64,128,27,54,108,216,171,77,154};

    // Multiplication tables for MixColumns
    int mul2[256];
    int mul3[256];

    // Expanded key (15 round keys, each 16 bytes)
    int roundKey[15][16];

    // Helper: fill multiplication tables
    void buildMulTables() {
        for (int i = 0; i < 256; ++i) {
            int x = i << 1;
            mul2[i] = (x ^ ((x & 0x100) ? 0x11B : 0)) & 0xFF;
            mul3[i] = mul2[i] ^ i;
        }
    }

    // Key expansion for 256‑bit key
    void expandKey(const int* key) {
        int w[60][4];                     // 60 words, each 4 bytes
        // copy initial 8 words (32 bytes)
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 4; ++j)
                w[i][j] = key[4*i + j] & 0xFF;

        for (int i = 8; i < 60; ++i) {
            int temp[4];
            for (int j = 0; j < 4; ++j) temp[j] = w[i-1][j];
            if (i % 8 == 0) {
                // RotWord
                int t = temp[0]; temp[0] = temp[1]; temp[1] = temp[2]; temp[2] = temp[3]; temp[3] = t;
                // SubWord
                for (int j = 0; j < 4; ++j) temp[j] = sbox[temp[j]];
                // Rcon
                temp[0] ^= rcon[i/8];
            } else if (i % 8 == 4) {
                for (int j = 0; j < 4; ++j) temp[j] = sbox[temp[j]];
            }
            for (int j = 0; j < 4; ++j)
                w[i][j] = w[i-8][j] ^ temp[j];
        }

        // pack round keys
        for (int r = 0; r < 15; ++r)
            for (int i = 0; i < 16; ++i)
                roundKey[r][i] = w[4*r + i/4][i%4] & 0xFF;
    }

    // SubBytes
    void subBytes(int* state) {
        for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];
    }

    // ShiftRows (in‑place)
    void shiftRows(int* s) {
        int t;
        // Row 1 (shift left 1)
        t = s[1]; s[1] = s[5]; s[5] = s[9]; s[9] = s[13]; s[13] = t;
        // Row 2 (shift left 2)
        t = s[2]; s[2] = s[10]; s[10] = t;
        t = s[6]; s[6] = s[14]; s[14] = t;
        // Row 3 (shift left 3)
        t = s[3]; s[3] = s[15]; s[15] = s[11]; s[11] = s[7]; s[7] = t;
    }

    // MixColumns
    void mixColumns(int* s) {
        for (int c = 0; c < 4; ++c) {
            int i0 = 4*c, i1 = i0+1, i2 = i0+2, i3 = i0+3;
            int a0 = s[i0], a1 = s[i1], a2 = s[i2], a3 = s[i3];
            s[i0] = mul2[a0] ^ mul3[a1] ^ a2 ^ a3;
            s[i1] = a0 ^ mul2[a1] ^ mul3[a2] ^ a3;
            s[i2] = a0 ^ a1 ^ mul2[a2] ^ mul3[a3];
            s[i3] = mul3[a0] ^ a1 ^ a2 ^ mul2[a3];
        }
    }

    // AddRoundKey
    void addRoundKey(int* s, int round) {
        for (int i = 0; i < 16; ++i) s[i] ^= roundKey[round][i];
    }

public:
    AES256() { buildMulTables(); }

    // Encrypt single 16‑byte block
    void encryptBlock(const int* in, const int* key, int* out) {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = in[i] & 0xFF;
        expandKey(key);
        addRoundKey(state, 0);
        int r = 1;
        while (r < 14) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, r);
            ++r;
        }
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 14);
        for (int i = 0; i < 16; ++i) out[i] = state[i] & 0xFF;
    }
};

int main() {
    // 16‑byte plaintext (mixed signed extremes)
    int plain[16] = {
        0,          // zero
        127,        // max positive byte
        -128,       // min negative byte
        0x55,       // positive 85
        -1,         // -1 -> 0xFF after masking
        0,          // zero
        0xFF,       // 255 positive
        0x33,       // 51
        -64,        // -64 -> 0xC0
        0x7A,       // 122
        0,          // zero
        -100,       // -100 -> 0x9C
        50,         // 50
        -50,        // -50 -> 0xCE
        0x0F,       // 15
        -15         // -15 -> 0xF1
    };

    // 32‑byte key (mixed signed extremes)
    int key[32] = {
        0x00, -0x01, 0x7F, 0x80,
        -0x7F, 0x01, 0xFF, -0xFF,
        0x55, -0x55, 0xAA, -0xAA,
        0x33, -0x33, 0xCC, -0xCC,
        0x11, -0x11, 0x22, -0x22,
        0x44, -0x44, 0x88, -0x88,
        0x99, -0x99, 0x66, -0x66,
        0x00, -0x00, 0x7E, -0x7E
    };

    int cipher[16];
    AES256 aes;
    aes.encryptBlock(plain, key, cipher);

    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xFF);
    std::cout << std::dec << std::endl;
    return 0;
}
