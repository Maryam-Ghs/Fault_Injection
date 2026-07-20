#include <cstdio>
#include <cstdlib>

/* LLM input variant 5: duplicate-heavy */

int mul2(int x) {
    int y = x << 1;
    if (y & 0x100) y ^= 0x11b;          // AES‑like reduction
    return y & 0xff;
}
int mul3(int x) { return mul2(x) ^ x; }

int main() {
    /* -------------------------------------------------------------
       ARIA‑256 – version #7
       All data lives on the heap, everything is inside main().
       Only int is used (no double, long, unsigned, const).
       ------------------------------------------------------------- */

    /* ---------- S‑boxes (AES S‑box and its inverse) --------------- */
    int sbox1[256] = {
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    };
    int sbox2[256] = {
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    };

    /* ------------------- Round constants -------------------------- */
    int rc[4][16] = {
        {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f},
        {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f},
        {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f},
        {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f}
    };

    /* ------------------- Input (heap) ----------------------------- */
    int* plain = new int[16];
    int* master = new int[32];
    for (int i = 0; i < 16; ++i) plain[i] = (i/2);          // 0,0,1,1,2,2,...
    for (int i = 0; i < 32; ++i) master[i] = ((i % 8) * 0x11) & 0xff; // repeating pattern with duplicates

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
