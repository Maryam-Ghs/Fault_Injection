/ * LLM input variant 3: zeros-and-ones * /
#include <iostream>
#include <iomanip>

int main() {
    // ---------- S‑box (heap) ----------
    int* sbox = new int[256];
    for (int i = 0; i < 256; ++i) sbox[i] = i % 2; // alternating zeros and ones

    // ---------- Rcon (heap) ----------
    int* rcon = new int[10];
    for (int i = 0; i < 10; ++i) rcon[i] = (i == 0) ? 1 : 0; // first is 1, rest zeros

    // ---------- Key and Plaintext (heap) ----------
    int* key = new int[16];
    for (int i = 0; i < 16; ++i) key[i] = i % 2; // alternating zeros and ones

    int* plain = new int[32];                     // two blocks
    for (int i = 0; i < 32; ++i) plain[i] = i % 2; // alternating zeros and ones

    // ---------- Round keys (heap) ----------
    int* roundKey = new int[176];                 // 11 * 16
    // copy original key
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
    for (int j = 0; j < 16; ++j) counter[j] = 0;   // start at 0

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
