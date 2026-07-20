/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <vector>
#include <iomanip>

//-------------------------------------------------------------------
//  Anubis cipher – version #7 (int / float only, branch‑minimized)
//-------------------------------------------------------------------

// 256‑entry S‑box (deterministic pseudo‑random permutation)
static int sbox[256] = {
    0, 7, 14, 21, 28, 35, 42, 49, 56, 63, 70, 77, 84, 91, 98,105,
    112,119,126,133,140,147,154,161,168,175,182,189,196,203,210,217,
    224,231,238,245,252,  3, 10, 17, 24, 31, 38, 45, 52, 59, 66, 73,
     80, 87, 94,101,108,115,122,129,136,143,150,157,164,171,178,185,
    192,199,206,213,220,227,234,241,248,255,  6, 13, 20, 27, 34, 41,
     48, 55, 62, 69, 76, 83, 90, 97,104,111,118,125,132,139,146,153,
    160,167,174,181,188,195,202,209,216,223,230,237,244,251,  2,  9,
     16, 23, 30, 37, 44, 51, 58, 65, 72, 79, 86, 93,100,107,114,121,
    128,135,142,149,156,163,170,177,184,191,198,205,212,219,226,233,
    240,247,254,  5, 12, 19, 26, 33, 40, 47, 54, 61, 68, 75, 82, 89,
     96,103,110,117,124,131,138,145,152,159,166,173,180,187,194,201,
    208,215,222,229,236,243,250,  1,  8, 15, 22, 29, 36, 43, 50, 57,
     64, 71, 78, 85, 92, 99,106,113,120,127,134,141,148,155,162,169,
    176,183,190,197,204,211,218,225,232,239,246,253,  4, 11, 18, 25,
     32, 39, 46, 53, 60, 67, 74, 81, 88, 95,102,109,116,123,130,137,
    144,151,158,165,172,179,186,193,200,207,214,221,228,235,242,249
};

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
    // placeholder: each column XOR of its four bytes → broadcast
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
// Main – generate deterministic test vectors, encrypt, print
//-------------------------------------------------------------------
int main() {
    // 128‑bit key – deterministic pseudo‑random pattern
    std::vector<int> key(16);
    for (int i = 0; i < 16; ++i)
        key[i] = ((i * 31) + 0x3C) & 0xFF;  // example pattern

    // 128‑bit plaintext – another deterministic pattern
    std::vector<int> plain(16);
    for (int i = 0; i < 16; ++i)
        plain[i] = ((i * 53) + 0x11) & 0xFF;

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
