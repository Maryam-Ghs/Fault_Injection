#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */

//-------------------------------------------------------------------
//  Anubis cipher – version #7 (int / float only, branch‑minimized)
//-------------------------------------------------------------------

// 256‑entry S‑box (int values, no const) – filled with a single repeated value
static int sbox[256];

// Initialize S‑box with duplicate‑heavy values
static void initSbox() {
    for (int i = 0; i < 256; ++i) {
        sbox[i] = 0x7F; // same value for every entry
    }
}

// round constants (int values, no const)
static int rcon[10] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36 };

//-------------------------------------------------------------------
// Helper: rotate a 4‑byte word left by one byte
//-------------------------------------------------------------------
static std::vector<int> rotWord(const std::vector<int>& w) {
    std::vector<int> r(4);
    r[0] = w[1]; r[1] = w[2]; r[2] = w[3]; r[3] = w[0];
    return r;
}

//-------------------------------------------------------------------
// Helper: apply S‑box to a 4‑byte word
//-------------------------------------------------------------------
static std::vector<int> subWord(const std::vector<int>& w) {
    std::vector<int> r(4);
    for (int i = 0; i < 4; ++i) {
        int idx = w[i];
        r[i] = sbox[idx];
    }
    return r;
}

//-------------------------------------------------------------------
// Key schedule – produces 11 round keys (128‑bit each)
//-------------------------------------------------------------------
static std::vector< std::vector<int> > expandKey(const std::vector<int>& key) {
    std::vector< std::vector<int> > rk(11, std::vector<int>(16));
    // first round key = original key
    for (int i = 0; i < 16; ++i) rk[0][i] = key[i];

    // generate subsequent round keys
    int round = 0;
    while (round < 10) {
        // take last 4‑byte word of previous round key
        std::vector<int> temp(4);
        for (int i = 0; i < 4; ++i) temp[i] = rk[round][12 + i];
        // rot + sub + rcon
        temp = rotWord(temp);
        temp = subWord(temp);
        temp[0] ^= rcon[round];
        // produce new round key word by word
        for (int i = 0; i < 4; ++i) {
            int base = i * 4;
            for (int j = 0; j < 4; ++j) {
                int a = rk[round][base + j];
                int b = (i == 0) ? temp[j] : rk[round+1][base - 4 + j];
                rk[round+1][base + j] = a ^ b;
            }
        }
        ++round;
    }
    return rk;
}

//-------------------------------------------------------------------
// SubBytes – apply S‑box to entire state
//-------------------------------------------------------------------
static void subBytes(std::vector<int>& st) {
    for (int i = 0; i < 16; ++i) {
        int idx = st[i];
        st[i] = sbox[idx];
    }
}

//-------------------------------------------------------------------
// ShiftRows – cyclic left shift per row (branch‑free)
//-------------------------------------------------------------------
static void shiftRows(std::vector<int>& st) {
    std::vector<int> tmp(16);
    for (int r = 0; r < 4; ++r) {
        int shift = r;
        for (int c = 0; c < 4; ++c) {
            int src = (c + shift) & 3;
            tmp[4 * r + c] = st[4 * r + src];
        }
    }
    st.swap(tmp);
}

//-------------------------------------------------------------------
// MixColumns – simplified (uses pre‑computed multiplication tables)
// For demonstration we use a very small placeholder transformation
//-------------------------------------------------------------------
static void mixColumns(std::vector<int>& st) {
    for (int c = 0; c < 4; ++c) {
        int a0 = st[0 + 4*c];
        int a1 = st[1 + 4*c];
        int a2 = st[2 + 4*c];
        int a3 = st[3 + 4*c];
        int mix = a0 ^ a1 ^ a2 ^ a3;
        st[0 + 4*c] = mix;
        st[1 + 4*c] = mix;
        st[2 + 4*c] = mix;
        st[3 + 4*c] = mix;
    }
}

//-------------------------------------------------------------------
// AddRoundKey – XOR state with round key
//-------------------------------------------------------------------
static void addRoundKey(std::vector<int>& st, const std::vector<int>& rk) {
    for (int i = 0; i < 16; ++i) {
        st[i] ^= rk[i];
    }
}

//-------------------------------------------------------------------
// Encrypt a single 16‑byte block
//-------------------------------------------------------------------
static std::vector<int> encryptBlock(const std::vector<int>& plain,
                                     const std::vector< std::vector<int> >& rkeys) {
    std::vector<int> state = plain;

    // initial AddRoundKey
    addRoundKey(state, rkeys[0]);

    int round = 1;
    while (round < 10) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, rkeys[round]);
        ++round;
    }

    // final round (no MixColumns)
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, rkeys[10]);

    return state;
}

//-------------------------------------------------------------------
// Main – generate duplicate‑heavy test vectors, encrypt, print
//-------------------------------------------------------------------
int main() {
    initSbox();

    // 128‑bit key – all bytes identical
    std::vector<int> key(16, 0x55); // 0x55 repeated

    // 128‑bit plaintext – all bytes identical
    std::vector<int> plain(16, 0x33); // 0x33 repeated

    // key expansion
    std::vector< std::vector<int> > roundKeys = expandKey(key);

    // encryption
    std::vector<int> cipher = encryptBlock(plain, roundKeys);

    // output as hex
    std::cout << "Plaintext : ";
    for (int v : plain) std::cout << std::hex << std::setw(2) << std::setfill('0') << v << ' ';
    std::cout << "\nCiphertext: ";
    for (int v : cipher) std::cout << std::hex << std::setw(2) << std::setfill('0') << v << ' ';
    std::cout << std::dec << std::endl;
    return 0;
}
