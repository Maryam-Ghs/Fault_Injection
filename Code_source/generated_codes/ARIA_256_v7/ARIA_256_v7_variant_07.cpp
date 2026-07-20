#include <cstdio>
#include <cstdlib>

int mul2(int x) {
    int y = x << 1;
    if (y & 0x100) y ^= 0x11b;          // AES‑like reduction
    return y & 0xff;
}
int mul3(int x) { return mul2(x) ^ x; }

int main() {
    /* LLM input variant 7: reverse-adversarial */
    /* -------------------------------------------------------------
       ARIA‑256 – version #7
       All data lives on the heap, everything is inside main().
       Only int is used (no double, long, unsigned, const).
       ------------------------------------------------------------- */

    /* ---------- S‑boxes (AES S‑box and its inverse) --------------- */
    int sbox1[256] = {
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
    int sbox2[256] = {
        82,9,106,213,48,54,165,56,191,64,163,158,129,243,215,251,
        124,227,57,130,155,47,255,135,52,142,67,68,196,222,233,203,
        84,123,148,50,166,194,35,61,238,76,149,11,66,250,195,78,
        8,46,161,102,40,217,36,178,118,91,162,73,109,139,209,37,
        114,248,246,100,134,104,152,22,212,164,92,204,93,101,182,146,
        108,112,72,80,253,237,185,218,94,21,70,87,167,141,157,132,
        144,216,171,0,140,188,211,10,247,228,88,5,184,179,69,6,
        208,44,30,143,202,63,15,2,193,175,189,3,1,19,138,107,
        58,145,17,65,79,103,220,234,151,242,207,206,240,180,230,115,
        150,172,116,34,231,173,53,133,226,249,55,232,28,117,223,110,
        71,241,26,113,29,41,197,137,111,183,98,14,170,24,190,27,
        252,86,62,75,198,210,121,32,154,219,192,254,120,205,90,244,
        31,221,168,51,136,7,199,49,177,18,16,89,39,128,236,95,
        96,81,127,169,25,181,74,13,45,229,122,159,147,201,156,239,
        160,224,59,77,174,42,245,176,200,235,187,60,131,83,153,97,
        23,43,4,126,186,119,214,38,225,105,20,99,85,33,12,125
    };

    /* ------------------- Round constants -------------------------- */
    int rc[4][16] = {
        {0x51,0x7c,0xc1,0xb7,0x27,0x22,0x0a,0x94,0xfe,0x13,0xab,0x5c,0x78,0x52,0x7c,0x10},
        {0x6d,0xb1,0xa9,0x3d,0x80,0x46,0x4e,0x12,0x3c,0x0c,0xd4,0x08,0xf2,0x88,0x14,0x4e},
        {0x02,0x6d,0x5b,0x31,0x1c,0x7a,0x2b,0x25,0x68,0x26,0x4a,0x3b,0x8c,0x03,0x5b,0x4c},
        {0x4a,0x5b,0x9b,0x2c,0x5a,0x5c,0x81,0x58,0x2d,0x72,0x3e,0x6e,0x0d,0x45,0x39,0x3a}
    };

    /* ------------------- Input (heap) ----------------------------- */
    int* plain = new int[16];
    int* master = new int[32];
    // Reverse‑ordered plaintext: 15 … 0
    for (int i = 0; i < 16; ++i) plain[i] = 15 - i;
    // Pseudo‑key with high byte values (inverse of original pattern)
    for (int i = 0; i < 32; ++i) master[i] = 0xff - ((i * 3) & 0xff);

    /* ------------------- Round key generation --------------------- */
    int* rkey = new int[13 * 16];   // 13 round keys (0 … 12)

    // key0 = K0 xor K1 (initial whitening key)
    for (int i = 0; i < 16; ++i)
        rkey[i] = master[i] ^ master[i + 16];

    // temporary buffer for key schedule
    int* tmp = new int[16];

    for (int round = 1; round <= 12; ++round) {
        // 1) add round constant
        int rcIdx = (round - 1) % 4;
        for (int i = 0; i < 16; ++i)
            tmp[i] = rkey[(round - 1) * 16 + i] ^ rc[rcIdx][i];

        // 2) substitution layer (alternating S‑boxes)
        if (round % 2 == 1) {
            for (int i = 0; i < 16; ++i) tmp[i] = sbox1[tmp[i]];
        } else {
            for (int i = 0; i < 16; ++i) tmp[i] = sbox2[tmp[i]];
        }

        // 3) diffusion (MixColumns‑like)
        for (int col = 0; col < 4; ++col) {
            int base = col * 4;
            int a0 = tmp[base];
            int a1 = tmp[base + 1];
            int a2 = tmp[base + 2];
            int a3 = tmp[base + 3];
            int b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            tmp[base]     = b0 & 0xff;
            tmp[base + 1] = b1 & 0xff;
            tmp[base + 2] = b2 & 0xff;
            tmp[base + 3] = b3 & 0xff;
        }

        // 4) generate next round key as XOR of previous key and transformed tmp
        for (int i = 0; i < 16; ++i)
            rkey[round * 16 + i] = rkey[(round - 1) * 16 + i] ^ tmp[i];
    }

    delete[] tmp;   // no longer needed

    /* ------------------- Encryption ------------------------------- */
    int* state = new int[16];
    // initial whitening
    for (int i = 0; i < 16; ++i) state[i] = plain[i] ^ rkey[i];

    // 11 full rounds (0 … 10)
    for (int rnd = 1; rnd <= 11; ++rnd) {
        // substitution (alternating)
        if (rnd % 2 == 1) {
            for (int i = 0; i < 16; ++i) state[i] = sbox1[state[i]];
        } else {
            for (int i = 0; i < 16; ++i) state[i] = sbox2[state[i]];
        }

        // diffusion
        for (int col = 0; col < 4; ++col) {
            int base = col * 4;
            int a0 = state[base];
            int a1 = state[base + 1];
            int a2 = state[base + 2];
            int a3 = state[base + 3];
            int b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            state[base]     = b0 & 0xff;
            state[base + 1] = b1 & 0xff;
            state[base + 2] = b2 & 0xff;
            state[base + 3] = b3 & 0xff;
        }

        // add round key
        for (int i = 0; i < 16; ++i) state[i] ^= rkey[rnd * 16 + i];
    }

    // final round (no diffusion)
    for (int i = 0; i < 16; ++i) state[i] = sbox1[state[i]];
    for (int i = 0; i < 16; ++i) state[i] ^= rkey[12 * 16 + i];

    /* ------------------- Output ----------------------------------- */
    for (int i = 0; i < 16; ++i)
        printf("%02x ", state[i] & 0xff);
    printf("\n");

    /* ------------------- Cleanup --------------------------------- */
    delete[] plain;
    delete[] master;
    delete[] rkey;
    delete[] state;

    return 0;
}
