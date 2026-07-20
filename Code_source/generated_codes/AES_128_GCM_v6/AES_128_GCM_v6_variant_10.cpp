#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */
int main() {
    /*------------------- 1.  Data Setup (edge‑case heavy) -------------------*/
    /* 128‑bit key (all zero) */
    std::vector<int> key(16);
    for (int i = 0; i < 16; ++i) key[i] = 0x00;

    /* 96‑bit IV (all zero) – the most common edge case */
    std::vector<int> iv(12);
    for (int i = 0; i < 12; ++i) iv[i] = 0x00;

    /* Plaintext – relatively large but safe (1024 bytes) */
    std::vector<int> pt(1024);
    for (int i = 0; i < 1024; ++i) pt[i] = i & 0xFF;

    /* AAD – 256 bytes of 0xAA (larger stress input) */
    std::vector<int> aad(256);
    for (int i = 0; i < 256; ++i) aad[i] = 0xAA;

    /*------------------- 2.  AES‑128 Core (manual unrolling) ---------------*/
    /* S‑box */
    std::vector<int> sbox = {
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

    /* Rcon */
    std::vector<int> rcon = {1,2,4,8,16,32,64,128,27,54};

    /* Key expansion – 44 words (4‑byte each) -> 176 bytes */
    std::vector<int> roundKey(176);
    for (int i = 0; i < 16; ++i) roundKey[i] = key[i];

    for (int i = 1; i <= 10; ++i) {
        /* ---- 4‑byte temp ---- */
        int t0 = roundKey[(i-1)*16 + 12];
        int t1 = roundKey[(i-1)*16 + 13];
        int t2 = roundKey[(i-1)*16 + 14];
        int t3 = roundKey[(i-1)*16 + 15];

        /* RotWord */
        int tmp = t0; t0 = t1; t1 = t2; t2 = t3; t3 = tmp;

        /* SubWord */
        t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];

        /* Rcon */
        t0 ^= rcon[i-1];

        for (int j = 0; j < 4; ++j) {
            int src = (i-1)*16 + j*4;
            int dst = i*16 + j*4;
            roundKey[dst + 0] = roundKey[src + 0] ^ t0;
            roundKey[dst + 1] = roundKey[src + 1] ^ t1;
            roundKey[dst + 2] = roundKey[src + 2] ^ t2;
            roundKey[dst + 3] = roundKey[src + 3] ^ t3;
            t0 = roundKey[dst + 0];
            t1 = roundKey[dst + 1];
            t2 = roundKey[dst + 2];
            t3 = roundKey[dst + 3];
        }
    }

    /* AES encrypt one block – lambda, fully unrolled where reasonable */
    auto aes_enc = [&](const std::vector<int>& in, std::vector<int>& out) {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = in[i] ^ roundKey[i];   // AddRoundKey(0)

        for (int round = 1; round < 10; ++round) {
            /* SubBytes */
            for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];

            /* ShiftRows (manual) */
            int tmp;

            /* Row 1 (shift 1) */
            tmp = state[1]; state[1] = state[5]; state[5] = state[9]; state[9] = state[13]; state[13] = tmp;
            /* Row 2 (shift 2) */
            tmp = state[2]; state[2] = state[10]; state[10] = tmp;
            tmp = state[6]; state[6] = state[14]; state[14] = tmp;
            /* Row 3 (shift 3) */
            tmp = state[3]; state[3] = state[15]; state[15] = state[11]; state[11] = state[7]; state[7] = tmp;

            /* MixColumns – use xtime macro style */
            for (int c = 0; c < 4; ++c) {
                int i0 = c*4+0, i1 = c*4+1, i2 = c*4+2, i3 = c*4+3;
                int s0 = state[i0], s1 = state[i1], s2 = state[i2], s3 = state[i3];
                int h = s0 ^ s1 ^ s2 ^ s3;
                int x0 = s0 ^ s1; x0 = ((x0 << 1) ^ ((x0 >> 7) * 0x1B)) & 0xFF;
                int x1 = s1 ^ s2; x1 = ((x1 << 1) ^ ((x1 >> 7) * 0x1B)) & 0xFF;
                int x2 = s2 ^ s3; x2 = ((x2 << 1) ^ ((x2 >> 7) * 0x1B)) & 0xFF;
                int x3 = s3 ^ s0; x3 = ((x3 << 1) ^ ((x3 >> 7) * 0x1B)) & 0xFF;
                state[i0] = s0 ^ x0 ^ h;
                state[i1] = s1 ^ x1 ^ h;
                state[i2] = s2 ^ x2 ^ h;
                state[i3] = s3 ^ x3 ^ h;
            }

            /* AddRoundKey */
            int rkoff = round*16;
            for (int i = 0; i < 16; ++i) state[i] ^= roundKey[rkoff + i];
        }

        /* Final round (no MixColumns) */
        for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];
        /* ShiftRows final */
        int tmp;
        tmp = state[1]; state[1] = state[5]; state[5] = state[9]; state[9] = state[13]; state[13] = tmp;
        tmp = state[2]; state[2] = state[10]; state[10] = tmp;
        tmp = state[6]; state[6] = state[14]; state[14] = tmp;
        tmp = state[3]; state[3] = state[15]; state[15] = state[11]; state[11] = state[7]; state[7] = tmp;

        int rkoff = 10*16;
        for (int i = 0; i < 16; ++i) out[i] = state[i] ^ roundKey[rkoff + i];
    };

    /*------------------- 3.  GCM Helpers -----------------------------------*/
    /* GF(2^128) multiplication (bit‑by‑bit) – vectors of 16 ints (big‑endian) */
    auto gmul = [&](const std::vector<int>& X, const std::vector<int>& Y) {
        std::vector<int> Z(16,0);
        std::vector<int> V = Y;                     // copy
        for (int i = 0; i < 128; ++i) {
            int byteIdx = i / 8;
            int bitIdx  = 7 - (i % 8);
            if ((X[byteIdx] >> bitIdx) & 1) {
                for (int j = 0; j < 16; ++j) Z[j] ^= V[j];
            }
            /* shift V right by 1 */
            int lsb = V[15] & 1;
            for (int j = 15; j > 0; --j) V[j] = (V[j] >> 1) | ((V[j-1] & 1) << 7);
            V[0] >>= 1;
            if (lsb) {
                V[0] ^= 0xE1;   // reduction polynomial 0xE1 for the MSB
            }
        }
        return Z;
    };

    /* GHASH – takes H, AAD, ciphertext */
    auto ghash = [&](const std::vector<int>& H,
                     const std::vector<int>& A,
                     const std::vector<int>& C) {
        std::vector<int> Y(16,0);
        /* process AAD */
        int aBlocks = (int)A.size() / 16;
        for (int i = 0; i < aBlocks; ++i) {
            for (int j = 0; j < 16; ++j) Y[j] ^= A[i*16 + j];
            Y = gmul(Y, H);
        }
        int aRem = (int)A.size() % 16;
        if (aRem) {
            std::vector<int> block(16,0);
            for (int j = 0; j < aRem; ++j) block[j] = A[aBlocks*16 + j];
            for (int j = 0; j < 16; ++j) Y[j] ^= block[j];
            Y = gmul(Y, H);
        }

        /* process ciphertext */
        int cBlocks = (int)C.size() / 16;
        for (int i = 0; i < cBlocks; ++i) {
            for (int j = 0; j < 16; ++j) Y[j] ^= C[i*16 + j];
            Y = gmul(Y, H);
        }
        int cRem = (int)C.size() % 16;
        if (cRem) {
            std::vector<int> block(16,0);
            for (int j = 0; j < cRem; ++j) block[j] = C[cBlocks*16 + j];
            for (int j = 0; j < 16; ++j) Y[j] ^= block[j];
            Y = gmul(Y, H);
        }

        /* length block (64‑bit AAD len || 64‑bit ciphertext len) */
        std::vector<int> lenBlock(16,0);
        long long aBits = (long long)A.size() * 8;   // still fits in 64‑bit
        long long cBits = (long long)C.size() * 8;
        for (int i = 0; i < 8; ++i) {
            lenBlock[7-i] = (int)((aBits >> (i*8)) & 0xFF);
            lenBlock[15-i] = (int)((cBits >> (i*8)) & 0xFF);
        }
        for (int j = 0; j < 16; ++j) Y[j] ^= lenBlock[j];
        Y = gmul(Y, H);
        return Y;
    };

    /*------------------- 4.  GCM Encryption --------------------------------*/
    /* H = AES‑encrypt(0^128) */
    std::vector<int> zeroBlock(16,0);
    std::vector<int> H(16);
    aes_enc(zeroBlock, H);

    /* J0 = IV || 0x00000001 (since IV is 12 bytes) */
    std::vector<int> J0 = iv;
    J0.push_back(0x00); J0.push_back(0x00); J0.push_back(0x00); J0.push_back(0x01);

    /* Counter mode – generate ciphertext */
    std::vector<int> ctr = J0;                // start from J0
    std::vector<int> ct;                     // ciphertext accumulator
    int blocks = (int)(pt.size() + 15) / 16;  // number of 16‑byte blocks

    for (int b = 0; b < blocks; ++b) {
        /* increment counter (big‑endian) */
        for (int i = 15; i >= 12; --i) {
            ctr[i] = (ctr[i] + 1) & 0xFF;
            if (ctr[i] != 0) break;
        }
        std::vector<int> keystream(16);
        aes_enc(ctr, keystream);
        int off = b*16;
        int chunk = (int)pt.size() - off;
        if (chunk > 16) chunk = 16;
        for (int i = 0; i < chunk; ++i) {
            ct.push_back(pt[off + i] ^ keystream[i]);
        }
    }

    /* GHASH over AAD and ciphertext */
    std::vector<int> S = ghash(H, aad, ct);

    /* Tag = AES‑encrypt(J0) XOR S */
    std::vector<int> E0(16);
    aes_enc(J0, E0);
    std::vector<int> tag(16);
    for (int i = 0; i < 16; ++i) tag[i] = E0[i] ^ S[i];

    /*------------------- 5.  Output ----------------------------------------*/
    std::cout << "Ciphertext (hex): ";
    for (size_t i = 0; i < ct.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << ct[i];
    }
    if (ct.empty()) std::cout << "(empty)";
    std::cout << std::dec << "\n";

    std::cout << "Tag (hex): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << tag[i];
    }
    std::cout << std::dec << "\n";

    return 0;
}
