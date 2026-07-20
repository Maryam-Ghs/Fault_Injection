#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

int main() {
    /* ---------- parameters ----------
       plaintext size (multiple of 16)
       key length 32 bytes (256‑bit)
       iv length 16 bytes
    */
    int plainLen = 65536;                    // larger safe stress test
    int *dataBuf = new int[plainLen];
    int *outBuf  = new int[plainLen];
    int *keyBuf  = new int[32];
    int *ivBuf   = new int[16];

    /* ---------- deterministic pseudo‑random source ---------- */
    std::srand(0);  // fixed seed for reproducibility
    for (int i = 0; i < plainLen; ++i) dataBuf[i] = std::rand() & 0xFF;
    for (int i = 0; i < 32; ++i)       keyBuf[i] = std::rand() & 0xFF;
    for (int i = 0; i < 16; ++i)       ivBuf[i]  = std::rand() & 0xFF;

    /* ---------- AES tables (int, no const) ---------- */
    int sBox[256] = {
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
    int rCon[15] = {1,2,4,8,16,32,64,128,27,54,108,216,171,77,154};

    /* ---------- multiplication tables for MixColumns ---------- */
    int mul2[256]; int mul3[256];
    for (int i = 0; i < 256; ++i) {
        int x = i << 1; if (x & 0x100) x ^= 0x11B; mul2[i] = x & 0xFF;
        mul3[i] = mul2[i] ^ i;
    }

    /* ---------- key expansion (AES‑256) ---------- */
    int *expKey = new int[240];                 // 15 round‑keys × 16 bytes
    for (int i = 0; i < 32; ++i) expKey[i] = keyBuf[i];
    for (int i = 8; i < 60; ++i) {
        int tmp0 = expKey[(i-1)*4+0];
        int tmp1 = expKey[(i-1)*4+1];
        int tmp2 = expKey[(i-1)*4+2];
        int tmp3 = expKey[(i-1)*4+3];
        if (i % 8 == 0) {
            int rot0 = tmp1, rot1 = tmp2, rot2 = tmp3, rot3 = tmp0;
            tmp0 = sBox[rot0]; tmp1 = sBox[rot1];
            tmp2 = sBox[rot2]; tmp3 = sBox[rot3];
            tmp0 ^= rCon[i/8 - 1];
        } else if (i % 8 == 4) {
            tmp0 = sBox[tmp0]; tmp1 = sBox[tmp1];
            tmp2 = sBox[tmp2]; tmp3 = sBox[tmp3];
        }
        int src = (i-8)*4;
        expKey[i*4+0] = expKey[src+0] ^ tmp0;
        expKey[i*4+1] = expKey[src+1] ^ tmp1;
        expKey[i*4+2] = expKey[src+2] ^ tmp2;
        expKey[i*4+3] = expKey[src+3] ^ tmp3;
    }

    /* ---------- lambda: encrypt one 16‑byte block ---------- */
    auto encryptBlock = [&](int *state) {
        int round = 0;
        /* AddRoundKey */
        for (int i = 0; i < 16; ++i) state[i] ^= expKey[round*16 + i];
        ++round;
        /* 13 full rounds */
        while (round < 14) {
            /* SubBytes */
            for (int i = 0; i < 16; ++i) state[i] = sBox[state[i]];
            /* ShiftRows (in‑place) */
            int tmp[16];
            for (int c = 0; c < 4; ++c) {
                int base = c*4;
                tmp[base+0] = state[base+0];
                tmp[base+1] = state[((c+1)&3)*4+1];
                tmp[base+2] = state[((c+2)&3)*4+2];
                tmp[base+3] = state[((c+3)&3)*4+3];
            }
            for (int i = 0; i < 16; ++i) state[i] = tmp[i];
            /* MixColumns */
            for (int c = 0; c < 4; ++c) {
                int i0 = c*4+0, i1 = c*4+1, i2 = c*4+2, i3 = c*4+3;
                int a0 = state[i0], a1 = state[i1], a2 = state[i2], a3 = state[i3];
                int r0 = mul2[a0] ^ mul3[a1] ^ a2 ^ a3;
                int r1 = a0 ^ mul2[a1] ^ mul3[a2] ^ a3;
                int r2 = a0 ^ a1 ^ mul2[a2] ^ mul3[a3];
                int r3 = mul3[a0] ^ a1 ^ a2 ^ mul2[a3];
                state[i0] = r0; state[i1] = r1; state[i2] = r2; state[i3] = r3;
            }
            /* AddRoundKey */
            for (int i = 0; i < 16; ++i) state[i] ^= expKey[round*16 + i];
            ++round;
        }
        /* Final round (no MixColumns) */
        for (int i = 0; i < 16; ++i) state[i] = sBox[state[i]];
        int tmp[16];
        for (int c = 0; c < 4; ++c) {
            int base = c*4;
            tmp[base+0] = state[base+0];
            tmp[base+1] = state[((c+1)&3)*4+1];
            tmp[base+2] = state[((c+2)&3)*4+2];
            tmp[base+3] = state[((c+3)&3)*4+3];
        }
        for (int i = 0; i < 16; ++i) state[i] = tmp[i];
        for (int i = 0; i < 16; ++i) state[i] ^= expKey[round*16 + i];
    };

    /* ---------- OFB mode ---------- */
    int *prev = new int[16];
    for (int i = 0; i < 16; ++i) prev[i] = ivBuf[i];
    int pos = 0;
    while (pos < plainLen) {
        int keystream[16];
        for (int i = 0; i < 16; ++i) keystream[i] = prev[i];
        encryptBlock(keystream);
        for (int i = 0; i < 16 && pos < plainLen; ++i, ++pos)
            outBuf[pos] = dataBuf[pos] ^ keystream[i];
        for (int i = 0; i < 16; ++i) prev[i] = keystream[i];
    }

    /* ---------- output ciphertext (hex) ---------- */
    for (int i = 0; i < plainLen; ++i) {
        int hi = (outBuf[i] >> 4) & 0xF;
        int lo = outBuf[i] & 0xF;
        std::cout << (char)(hi < 10 ? hi + '0' : hi - 10 + 'a');
        std::cout << (char)(lo < 10 ? lo + '0' : lo - 10 + 'a');
        if ((i+1) % 16 == 0) std::cout << '\n';
        else if ((i+1) % 8 == 0) std::cout << ' ';
    }

    delete [] dataBuf;
    delete [] outBuf;
    delete [] keyBuf;
    delete [] ivBuf;
    delete [] expKey;
    delete [] prev;
    return 0;
}
