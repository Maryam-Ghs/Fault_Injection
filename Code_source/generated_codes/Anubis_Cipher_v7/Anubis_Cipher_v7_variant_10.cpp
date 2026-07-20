#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

//-------------------------------------------------------------------
//  Anubis cipher – version #7 (int / float only, branch‑minimized)
//-------------------------------------------------------------------

// 256‑entry S‑box (int values, no const)
static int sbox[256] = {
    0xB6,0xE5,0xF0,0x5C,0x57,0xE4,0x9F,0xC8,0xFC,0xF8,0x7C,0x0A,0xFF,0xA3,0x8C,0x0B,
    0x5C,0xF9,0x6C,0xD8,0x5F,0xE7,0x45,0x7D,0x8A,0x3A,0x6F,0xE8,0x1E,0xE9,0xC6,0x3F,
    // Reuse the first 16 values repeatedly to fill the table
    0xB6,0xE5,0xF0,0x5C,0x57,0xE4,0x9F,0xC8,0xFC,0xF8,0x7C,0x0A,0xFF,0xA3,0x8C,0x0B,
    0x5C,0xF9,0x6C,0xD8,0x5F,0xE7,0x45,0x7D,0x8A,0x3A,0x6F,0xE8,0x1E,0xE9,0xC6,0x3F,
    // Fill the rest with a simple pattern to stay safe
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
    // Pad the remaining entries with zeros (still safe)
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
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
        int idx = w[i] & 0xFF;
        r[i] = sbox[idx];
    }
    return r;
}

//-------------------------------------------------------------------
// Key schedule – produces 11 round keys (128‑bit each)
//-------------------------------------------------------------------
static std::vector< std::vector<int> > expandKey(const std::vector<int>& key) {
    std::vector< std::vector<int> > rk(11, std::vector<int>(16));
    for (int i = 0; i < 16; ++i) rk[0][i] = key[i];

    int round = 0;
    while (round < 10) {
        std::vector<int> temp(4);
        for (int i = 0; i < 4; ++i) temp[i] = rk[round][12 + i];
        temp = rotWord(temp);
        temp = subWord(temp);
        temp[0] ^= rcon[round];
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
        int idx = st[i] & 0xFF;
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
    addRoundKey(state, rkeys[0]);

    int round = 1;
    while (round < 10) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, rkeys[round]);
        ++round;
    }

    subBytes(state);
    shiftRows(state);
    addRoundKey(state, rkeys[10]);

    return state;
}

//-------------------------------------------------------------------
// Main – generate a larger deterministic test vector, encrypt all blocks
//-------------------------------------------------------------------
int main() {
    // 128‑bit key (16 bytes) – simple incremental pattern
    std::vector<int> key(16);
    for (int i = 0; i < 16; ++i) key[i] = i;

    // Generate a larger plaintext: 1024 bytes (64 blocks)
    const int total_bytes = 1024;
    std::vector<int> plain(total_bytes);
    for (int i = 0; i < total_bytes; ++i) {
        plain[i] = (0x10 + i) & 0xFF; // simple wrap‑around pattern
    }

    // Expand key once
    std::vector< std::vector<int> > roundKeys = expandKey(key);

    // Encrypt each 16‑byte block
    std::vector<int> cipher(total_bytes);
    for (int offset = 0; offset < total_bytes; offset += 16) {
        std::vector<int> block(plain.begin() + offset, plain.begin() + offset + 16);
        std::vector<int> enc = encryptBlock(block, roundKeys);
        std::copy(enc.begin(), enc.end(), cipher.begin() + offset);
    }

    // Output plaintext and ciphertext as hexadecimal streams
    std::cout << "Plaintext (" << total_bytes << " bytes):\n";
    for (int i = 0; i < total_bytes; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i] << ' ';
        if ((i + 1) % 16 == 0) std::cout << '\n';
    }

    std::cout << "\nCiphertext (" << total_bytes << " bytes):\n";
    for (int i = 0; i < total_bytes; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << cipher[i] << ' ';
        if ((i + 1) % 16 == 0) std::cout << '\n';
    }

    std::cout << std::dec;
    return 0;
}
