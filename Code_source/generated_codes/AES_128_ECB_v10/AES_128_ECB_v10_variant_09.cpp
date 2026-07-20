#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <ctime>

/* LLM input variant 9: medium-deterministic-random */

// -----------------------------------------------------------------------------
//  AES-128 (ECB) implementation – version #10
//  Only signed int (and float) are used. No const, unsigned, double or long.
//  Helper functions are heavily loop‑based, with temporary variables.
// -----------------------------------------------------------------------------

// ----- S‑Box ---------------------------------------------------------------
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

// ----- Rcon ---------------------------------------------------------------
int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

// ----- Helper: multiply by 2 in GF(2^8) ------------------------------------
int xtime(int v) {
    int shifted = (v << 1) & 0xff;
    int reduced = (v >> 7) & 1;
    return (shifted ^ (reduced ? 0x1b : 0)) & 0xff;
}

// ----- Deterministic pseudo‑random generator -------------------------------
int deterministic_rand() {
    static unsigned int seed = 0xDEADBEEF;
    seed = (seed * 1664525u + 1013904223u) & 0xffffffffu;
    return (seed >> 24) & 0xff;
}

// ----- SubBytes ------------------------------------------------------------
void subBytes(std::vector<int> &st) {
    for (int i = 0; i < 16; ++i) {
        int tmp = st[i];
        st[i] = sbox[tmp];
    }
}

// ----- ShiftRows -----------------------------------------------------------
void shiftRows(std::vector<int> &st) {
    // row 1 (indexes 1,5,9,13) rotate left by 1
    int t1 = st[1];
    st[1] = st[5];
    st[5] = st[9];
    st[9] = st[13];
    st[13] = t1;

    // row 2 (indexes 2,6,10,14) rotate left by 2
    int t2 = st[2];
    int t6 = st[6];
    st[2] = st[10];
    st[6] = st[14];
    st[10] = t2;
    st[14] = t6;

    // row 3 (indexes 3,7,11,15) rotate left by 3 (right by 1)
    int t3 = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7] = st[3];
    st[3] = t3;
}

// ----- MixColumns ----------------------------------------------------------
void mixColumns(std::vector<int> &st) {
    for (int c = 0; c < 4; ++c) {
        int i0 = c * 4;
        int a0 = st[i0];
        int a1 = st[i0 + 1];
        int a2 = st[i0 + 2];
        int a3 = st[i0 + 3];

        int t = a0 ^ a1 ^ a2 ^ a3;

        int u = a0;
        int v = a0 ^ a1; v = xtime(v); st[i0] = a0 ^ v ^ t;
        v = a1 ^ a2; v = xtime(v); st[i0 + 1] = a1 ^ v ^ t;
        v = a2 ^ a3; v = xtime(v); st[i0 + 2] = a2 ^ v ^ t;
        v = a3 ^ u;  v = xtime(v); st[i0 + 3] = a3 ^ v ^ t;
    }
}

// ----- AddRoundKey ---------------------------------------------------------
void addRoundKey(std::vector<int> &st, const std::vector<int> &rk, int round) {
    int base = round * 16;
    for (int i = 0; i < 16; ++i) {
        st[i] ^= rk[base + i];
    }
}

// ----- Key Expansion -------------------------------------------------------
void keyExpand(const std::vector<int> &key, std::vector<int> &rk) {
    // first 16 bytes are the original key
    for (int i = 0; i < 16; ++i) rk[i] = key[i];

    int bytes = 16;
    int r = 1;
    while (bytes < 176) {
        // take previous 4‑byte word
        int w0 = rk[bytes - 4];
        int w1 = rk[bytes - 3];
        int w2 = rk[bytes - 2];
        int w3 = rk[bytes - 1];

        // every 16‑byte boundary apply core
        if (bytes % 16 == 0) {
            // RotWord
            int tmp = w0; w0 = w1; w1 = w2; w2 = w3; w3 = tmp;
            // SubWord
            w0 = sbox[w0]; w1 = sbox[w1]; w2 = sbox[w2]; w3 = sbox[w3];
            // Rcon
            w0 ^= rcon[r];
            ++r;
        }

        // XOR with word 16 bytes before
        int p0 = rk[bytes - 16];
        int p1 = rk[bytes - 15];
        int p2 = rk[bytes - 14];
        int p3 = rk[bytes - 13];

        rk[bytes]     = p0 ^ w0; ++bytes;
        rk[bytes]     = p1 ^ w1; ++bytes;
        rk[bytes]     = p2 ^ w2; ++bytes;
        rk[bytes]     = p3 ^ w3; ++bytes;
    }
}

// ----- Encrypt one block ----------------------------------------------------
void encryptBlock(const std::vector<int> &in, const std::vector<int> &rk,
                  std::vector<int> &out) {
    std::vector<int> state(16);
    for (int i = 0; i < 16; ++i) state[i] = in[i];

    addRoundKey(state, rk, 0);

    for (int round = 1; round <= 9; ++round) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, rk, round);
    }

    subBytes(state);
    shiftRows(state);
    addRoundKey(state, rk, 10);

    for (int i = 0; i < 16; ++i) out[i] = state[i];
}

// -----------------------------------------------------------------------------
//  Main – generate deterministic data, encrypt in ECB mode, print hex output.
// -----------------------------------------------------------------------------
int main() {
    // ----- deterministic 128‑bit key -----------------------------------------
    std::vector<int> key(16);
    for (int i = 0; i < 16; ++i) key[i] = deterministic_rand();

    // ----- expand key ---------------------------------------------------------
    std::vector<int> roundKeys(176);
    keyExpand(key, roundKeys);

    // ----- deterministic plaintext (e.g., 768 bytes) -------------------------
    int blocks = 48;                     // 48 * 16 = 768 bytes
    std::vector<int> plain(blocks * 16);
    for (int i = 0; i < blocks * 16; ++i) plain[i] = deterministic_rand();

    // ----- encrypt each block -------------------------------------------------
    std::vector<int> cipher(blocks * 16);
    std::vector<int> tmpIn(16), tmpOut(16);
    for (int b = 0; b < blocks; ++b) {
        for (int i = 0; i < 16; ++i) tmpIn[i] = plain[b * 16 + i];
        encryptBlock(tmpIn, roundKeys, tmpOut);
        for (int i = 0; i < 16; ++i) cipher[b * 16 + i] = tmpOut[i];
    }

    // ----- print ciphertext as hex -------------------------------------------
    for (int i = 0; i < blocks * 16; ++i) {
        if (i % 16 == 0) std::cout << "\nBlock " << i / 16 << ": ";
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipher[i] & 0xff) << " ";
    }
    std::cout << std::dec << std::endl;
    return 0;
}
