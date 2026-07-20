#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 6: ordered-structured */

int main()
{
    // -------------------------------------------------------------
    // Version 10 – ARIA‑CBC‑256 (all code lives inside main)
    // -------------------------------------------------------------

    // ----- helpers ------------------------------------------------
    // simple S‑boxes (real ARIA uses fixed tables – here we mimic them)
    static int S1[256];
    static int S2[256];
    for (int i = 0; i < 256; ++i) {
        // a cheap reversible mapping, good enough for a demo
        S1[i] = ((i * 0x5a) ^ 0x3c) & 0xff;
        S2[i] = ((i * 0x7d) ^ 0xa1) & 0xff;
    }

    // round constants (ARIA‑256 uses 12 rounds + 2 whitening steps)
    static int RC[13][16];
    for (int r = 0; r < 13; ++r) {
        for (int i = 0; i < 16; ++i) {
            RC[r][i] = (r * 0x11 + i * 0x23) & 0xff;
        }
    }

    // ----- generate deterministic key, IV and plaintext -----------------
    // deterministic, ordered data instead of random
    std::vector<int> masterKey(32);
    for (int i = 0; i < 32; ++i) masterKey[i] = i & 0xff;          // 0,1,2,...

    std::vector<int> iv(16);
    for (int i = 0; i < 16; ++i) iv[i] = (i + 32) & 0xff;          // 32..47

    const int BLOCKS = 1024;               // large ordered array
    std::vector<int> plain(BLOCKS * 16);
    for (int i = 0; i < BLOCKS * 16; ++i) plain[i] = i & 0xff;    // 0,1,2,...

    // ----- key schedule (very simplified – just rotates) ----------
    std::vector< std::vector<int> > roundKey(13, std::vector<int>(16));
    // whitening keys (0 and 12) are direct copies of the master key halves
    for (int i = 0; i < 16; ++i) roundKey[0][i] = masterKey[i];
    for (int i = 0; i < 16; ++i) roundKey[12][i] = masterKey[16 + i];

    // generate middle round keys by rotating and xoring with RC
    for (int r = 1; r < 12; ++r) {
        for (int i = 0; i < 16; ++i) {
            int src = (i + r) & 0x1f;                 // rotate inside 32‑byte key
            int byte = (src < 16) ? masterKey[src] : masterKey[src - 16];
            roundKey[r][i] = (byte ^ RC[r][i]) & 0xff;
        }
    }

    // ----- CBC encryption -----------------------------------------
    std::vector<int> cipher(BLOCKS * 16);
    std::vector<int> prev(16);
    for (int i = 0; i < 16; ++i) prev[i] = iv[i];

    // manual unrolling of the 12‑round ARIA encryption
    for (int blk = 0; blk < BLOCKS; ++blk) {
        // ----- load block and apply CBC xor -----------------------
        int s0 = plain[blk * 16 + 0] ^ prev[0];
        int s1 = plain[blk * 16 + 1] ^ prev[1];
        int s2 = plain[blk * 16 + 2] ^ prev[2];
        int s3 = plain[blk * 16 + 3] ^ prev[3];
        int s4 = plain[blk * 16 + 4] ^ prev[4];
        int s5 = plain[blk * 16 + 5] ^ prev[5];
        int s6 = plain[blk * 16 + 6] ^ prev[6];
        int s7 = plain[blk * 16 + 7] ^ prev[7];
        int s8 = plain[blk * 16 + 8] ^ prev[8];
        int s9 = plain[blk * 16 + 9] ^ prev[9];
        int sA = plain[blk * 16 +10] ^ prev[10];
        int sB = plain[blk * 16 +11] ^ prev[11];
        int sC = plain[blk * 16 +12] ^ prev[12];
        int sD = plain[blk * 16 +13] ^ prev[13];
        int sE = plain[blk * 16 +14] ^ prev[14];
        int sF = plain[blk * 16 +15] ^ prev[15];

        // ----- round 0 (whitening) --------------------------------
        s0 = (s0 ^ roundKey[0][0]) & 0xff;  s1 = (s1 ^ roundKey[0][1]) & 0xff;
        s2 = (s2 ^ roundKey[0][2]) & 0xff;  s3 = (s3 ^ roundKey[0][3]) & 0xff;
        s4 = (s4 ^ roundKey[0][4]) & 0xff;  s5 = (s5 ^ roundKey[0][5]) & 0xff;
        s6 = (s6 ^ roundKey[0][6]) & 0xff;  s7 = (s7 ^ roundKey[0][7]) & 0xff;
        s8 = (s8 ^ roundKey[0][8]) & 0xff;  s9 = (s9 ^ roundKey[0][9]) & 0xff;
        sA = (sA ^ roundKey[0][10]) & 0xff; sB = (sB ^ roundKey[0][11]) & 0xff;
        sC = (sC ^ roundKey[0][12]) & 0xff; sD = (sD ^ roundKey[0][13]) & 0xff;
        sE = (sE ^ roundKey[0][14]) & 0xff; sF = (sF ^ roundKey[0][15]) & 0xff;

        // ----- rounds 1 … 11 (alternating SL1/SL2) ---------------
        for (int r = 1; r <= 11; ++r) {
            // substitution layer (odd rounds use S1, even use S2)
            if (r & 1) {
                s0 = S1[s0]; s1 = S1[s1]; s2 = S1[s2]; s3 = S1[s3];
                s4 = S1[s4]; s5 = S1[s5]; s6 = S1[s6]; s7 = S1[s7];
                s8 = S1[s8]; s9 = S1[s9]; sA = S1[sA]; sB = S1[sB];
                sC = S1[sC]; sD = S1[sD]; sE = S1[sE]; sF = S1[sF];
            } else {
                s0 = S2[s0]; s1 = S2[s1]; s2 = S2[s2]; s3 = S2[s3];
                s4 = S2[s4]; s5 = S2[s5]; s6 = S2[s6]; s7 = S2[s7];
                s8 = S2[s8]; s9 = S2[s9]; sA = S2[sA]; sB = S2[sB];
                sC = S2[sC]; sD = S2[sD]; sE = S2[sE]; sF = S2[sF];
            }

            // diffusion layer – 4‑byte matrix (fully unrolled)
            int t0 = (s0 ^ s4 ^ s8 ^ sC) & 0xff;
            int t1 = (s1 ^ s5 ^ s9 ^ sD) & 0xff;
            int t2 = (s2 ^ s6 ^ sA ^ sE) & 0xff;
            int t3 = (s3 ^ s7 ^ sB ^ sF) & 0xff;

            int t4 = (s0 ^ s5 ^ sA ^ sF) & 0xff;
            int t5 = (s1 ^ s6 ^ sB ^ sC) & 0xff;
            int t6 = (s2 ^ s7 ^ s8 ^ sD) & 0xff;
            int t7 = (s3 ^ s4 ^ s9 ^ sE) & 0xff;

            int t8  = (s0 ^ s6 ^ sB ^ sD) & 0xff;
            int t9  = (s1 ^ s7 ^ s8 ^ sF) & 0xff;
            int tA  = (s2 ^ s4 ^ s9 ^ sC) & 0xff;
            int tB  = (s3 ^ s5 ^ sA ^ sE) & 0xff;

            int tC = (s0 ^ s7 ^ s9 ^ sE) & 0xff;
            int tD = (s1 ^ s4 ^ sA ^ sF) & 0xff;
            int tE = (s2 ^ s5 ^ sB ^ sC) & 0xff;
            int tF = (s3 ^ s6 ^ s8 ^ sD) & 0xff;

            // add round key (xor)
            s0 = (t0 ^ roundKey[r][0]) & 0xff; s1 = (t1 ^ roundKey[r][1]) & 0xff;
            s2 = (t2 ^ roundKey[r][2]) & 0xff; s3 = (t3 ^ roundKey[r][3]) & 0xff;
            s4 = (t4 ^ roundKey[r][4]) & 0xff; s5 = (t5 ^ roundKey[r][5]) & 0xff;
            s6 = (t6 ^ roundKey[r][6]) & 0xff; s7 = (t7 ^ roundKey[r][7]) & 0xff;
            s8 = (t8 ^ roundKey[r][8]) & 0xff; s9 = (t9 ^ roundKey[r][9]) & 0xff;
            sA = (tA ^ roundKey[r][10]) & 0xff; sB = (tB ^ roundKey[r][11]) & 0xff;
            sC = (tC ^ roundKey[r][12]) & 0xff; sD = (tD ^ roundKey[r][13]) & 0xff;
            sE = (tE ^ roundKey[r][14]) & 0xff; sF = (tF ^ roundKey[r][15]) & 0xff;
        }

        // ----- final whitening (round 12) -------------------------
        s0 = (s0 ^ roundKey[12][0]) & 0xff;  s1 = (s1 ^ roundKey[12][1]) & 0xff;
        s2 = (s2 ^ roundKey[12][2]) & 0xff;  s3 = (s3 ^ roundKey[12][3]) & 0xff;
        s4 = (s4 ^ roundKey[12][4]) & 0xff;  s5 = (s5 ^ roundKey[12][5]) & 0xff;
        s6 = (s6 ^ roundKey[12][6]) & 0xff;  s7 = (s7 ^ roundKey[12][7]) & 0xff;
        s8 = (s8 ^ roundKey[12][8]) & 0xff;  s9 = (s9 ^ roundKey[12][9]) & 0xff;
        sA = (sA ^ roundKey[12][10]) & 0xff; sB = (sB ^ roundKey[12][11]) & 0xff;
        sC = (sC ^ roundKey[12][12]) & 0xff; sD = (sD ^ roundKey[12][13]) & 0xff;
        sE = (sE ^ roundKey[12][14]) & 0xff; sF = (sF ^ roundKey[12][15]) & 0xff;

        // ----- store ciphertext and update chaining vector ----------
        cipher[blk * 16 + 0] = s0; cipher[blk * 16 + 1] = s1;
        cipher[blk * 16 + 2] = s2; cipher[blk * 16 + 3] = s3;
        cipher[blk * 16 + 4] = s4; cipher[blk * 16 + 5] = s5;
        cipher[blk * 16 + 6] = s6; cipher[blk * 16 + 7] = s7;
        cipher[blk * 16 + 8] = s8; cipher[blk * 16 + 9] = s9;
        cipher[blk * 16 +10] = sA; cipher[blk * 16 +11] = sB;
        cipher[blk * 16 +12] = sC; cipher[blk * 16 +13] = sD;
        cipher[blk * 16 +14] = sE; cipher[blk * 16 +15] = sF;

        prev[0] = s0;  prev[1] = s1;  prev[2] = s2;  prev[3] = s3;
        prev[4] = s4;  prev[5] = s5;  prev[6] = s6;  prev[7] = s7;
        prev[8] = s8;  prev[9] = s9;  prev[10] = sA; prev[11] = sB;
        prev[12] = sC; prev[13] = sD; prev[14] = sE; prev[15] = sF;
    }

    // ----- output a sample of the ciphertext (hex) ----------------
    std::cout << "First 64 ciphertext bytes (hex):\n";
    for (int i = 0; i < 64; ++i) {
        int v = cipher[i];
        if (v < 16) std::cout << '0';
        std::cout << std::hex << v;
        if ((i + 1) % 16 == 0) std::cout << '\n';
        else std::cout << ' ';
    }
    return 0;
}
