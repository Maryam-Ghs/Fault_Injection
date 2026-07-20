#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */

//-------------------------------------------------------------------
//  Anubis cipher – version #7 (int / float only, branch‑minimized)
//-------------------------------------------------------------------

// 256‑entry S‑box (int values, reversed order for adversarial effect)
static int sbox[256] = {
    255,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,
    239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,
    223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,
    207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,
    191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,
    175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,
    159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,
    143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,
    127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,
    111,110,109,108,107,106,105,104,103,102,101,100, 99, 98, 97, 96,
     95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80,
     79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64,
     63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
     47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
     31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
     15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
};

// round constants (int values, reversed order)
static int rcon[10] = { 0x36,0x1B,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01 };

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
// MixColumns – simplified placeholder transformation
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
// Main – generate reverse‑ordered test vector, encrypt, print
//-------------------------------------------------------------------
int main() {
    // 128‑bit key – reverse order (15 down to 0)
    std::vector<int> key(16);
    for (int i = 0; i < 16; ++i) key[i] = 15 - i;

    // 128‑bit plaintext – reverse order (0x1F down to 0x10)
    std::vector<int> plain(16);
    for (int i = 0; i < 16; ++i) plain[i] = 0x1F - i;

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
