/* LLM input variant 7: reverse-adversarial */
// AES‑128‑CTR implementation – version #6
// All code lives inside main(), uses only int/float, heap allocation and loop‑heavy style.

#include <iostream>
#include <iomanip>

int main() {
    // ---------- S‑box (heap) ----------
    int* sbox = new int[256];
    int sb[256] = {
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
    for (int i = 0; i < 256; ++i) sbox[i] = sb[i];

    // ---------- Rcon (heap) ----------
    int* rcon = new int[10];
    int rc[10] = {1,2,4,8,16,32,64,128,27,54};
    for (int i = 0; i < 10; ++i) rcon[i] = rc[i];

    // ---------- Key and Plaintext (heap) ----------
    int* key = new int[16];
    // reversed key (adversarial ordering)
    int k[16] = {0x3c,0x4f,0xcf,0x09,0x88,0x15,0xf7,0xab,0xa6,0xd2,0xae,0x28,0x16,0x15,0x7e,0x2b};
    for (int i = 0; i < 16; ++i) key[i] = k[i];

    int* plain = new int[32];                     // two blocks
    // reversed plaintext (adversarial ordering)
    int p[32] = {
        0x51,0x8e,0xaf,0x45,0xac,0x6f,0xb7,0x9e,
        0x9c,0xac,0x03,0x1e,0x57,0x8a,0x2d,0xae,
        0x2a,0x17,0x93,0x73,0x11,0x7e,0x3d,0xe9,
        0x96,0x9f,0x40,0x2e,0xe2,0xbe,0xc1,0x6b
    };
    for (int i = 0; i < 32; ++i) plain[i] = p[i];

    // ---------- Round keys (heap) ----------
    int* roundKey = new int[176];                 // 11 * 16
    // copy original (reversed) key
    for (int i = 0; i < 16; ++i) roundKey[i] = key[i];

    // ---------- Key Expansion ----------
    int i = 16;
    while (i < 176) {
        int temp0 = roundKey[i-4];
        int temp1 = roundKey[i-3];
        int temp2 = roundKey[i-2];
        int temp3 = roundKey[i-1];

        if ((i/4) % 4 == 0) {
            // RotWord
            int t = temp0;
            temp0 = temp1; temp1 = temp2; temp2 = temp3; temp3 = t;
            // SubWord
            temp0 = sbox[temp0];
            temp1 = sbox[temp1];
            temp2 = sbox[temp2];
            temp3 = sbox[temp3];
            // Rcon
            temp0 = (temp0 ^ rcon[(i/16)-1]) & 0xff;
        }

        roundKey[i]   = (roundKey[i-16] ^ temp0) & 0xff;
        roundKey[i+1] = (roundKey[i-15] ^ temp1) & 0xff;
        roundKey[i+2] = (roundKey[i-14] ^ temp2) & 0xff;
        roundKey[i+3] = (roundKey[i-13] ^ temp3) & 0xff;
        i += 4;
    }

    // ---------- Helper lambdas ----------
    auto xtime = [&](int x) -> int {
        return ((x << 1) ^ ((x >> 7) & 1) * 0x1b) & 0xff;
    };

    auto subBytes = [&](int* st) {
        for (int j = 0; j < 16; ++j) st[j] = sbox[st[j]];
    };

    auto shiftRows = [&](int* st) {
        int tmp[16];
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                int src = (c + r) % 4;
                tmp[4*c + r] = st[4*src + r];
            }
        }
        for (int j = 0; j < 16; ++j) st[j] = tmp[j];
    };

    auto mixColumns = [&](int* st) {
        for (int c = 0; c < 4; ++c) {
            int a0 = st[4*c + 0];
            int a1 = st[4*c + 1];
            int a2 = st[4*c + 2];
            int a3 = st[4*c + 3];
            int t = a0 ^ a1 ^ a2 ^ a3;
            int u = a0;
            st[4*c + 0] = (a0 ^ t ^ xtime(a0 ^ a1)) & 0xff;
            st[4*c + 1] = (a1 ^ t ^ xtime(a1 ^ a2)) & 0xff;
            st[4*c + 2] = (a2 ^ t ^ xtime(a2 ^ a3)) & 0xff;
            st[4*c + 3] = (a3 ^ t ^ xtime(a3 ^ u )) & 0xff;
        }
    };

    auto addRoundKey = [&](int* st, int round) {
        int offset = round * 16;
        for (int j = 0; j < 16; ++j) st[j] = (st[j] ^ roundKey[offset + j]) & 0xff;
    };

    // ---------- CTR mode ----------
    int* counter = new int[16];
    // start counter at maximum value (adversarial)
    for (int j = 0; j < 16; ++j) counter[j] = 0xff;

    int* cipher = new int[32];

    int blockIdx = 0;
    while (blockIdx < 2) {
        // ---- encrypt counter ----
        int* state = new int[16];
        for (int j = 0; j < 16; ++j) state[j] = counter[j];

        addRoundKey(state, 0);
        int round = 1;
        while (round < 10) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
            ++round;
        }
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 10);                     // final round

        // ---- XOR with plaintext ----
        for (int j = 0; j < 16; ++j) {
            cipher[blockIdx*16 + j] = (plain[blockIdx*16 + j] ^ state[j]) & 0xff;
        }

        // ---- increment counter (big‑endian) ----
        int idx = 15;
        while (idx >= 0) {
            counter[idx] = (counter[idx] + 1) & 0xff;
            if (counter[idx] != 0) break;
            --idx;
        }

        delete[] state;
        ++blockIdx;
    }

    // ---------- Output ----------
    std::cout << std::hex << std::setfill('0');
    for (int i = 0; i < 32; ++i) {
        std::cout << std::setw(2) << (cipher[i] & 0xff);
        if (i % 16 == 15) std::cout << std::endl;
        else std::cout << ' ';
    }

    // ---------- Cleanup ----------
    delete[] sbox;
    delete[] rcon;
    delete[] key;
    delete[] plain;
    delete[] roundKey;
    delete[] counter;
    delete[] cipher;

    return 0;
}
