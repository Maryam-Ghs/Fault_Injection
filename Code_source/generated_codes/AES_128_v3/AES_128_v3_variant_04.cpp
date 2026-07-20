/* LLM input variant 4: signed-extremes */
// AES-128 implementation – version #3
// No use of double, long, unsigned, const. Only int and std::vector.
// Edge‑case inputs are encrypted and printed.

#include <iostream>
#include <vector>
#include <iomanip>

// ----- Helper tables -------------------------------------------------
std::vector<int> sbox = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

std::vector<int> rcon = {
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36
};

// ----- Utility -------------------------------------------------------
int xtime(int v) {
    int shifted = (v << 1) & 0xFF;
    int reduced = ((v >> 7) & 1) * 0x1b;
    return shifted ^ reduced;
}

// ----- Core AES operations -------------------------------------------
void SubBytes(std::vector<int>& st) {
    for (int i = 0; i < 16; ++i) {
        st[i] = sbox[st[i]];
    }
}

void ShiftRows(std::vector<int>& st) {
    // Row 1 (index 1,5,9,13) rotate left by 1
    int t1 = st[1];
    st[1] = st[5];
    st[5] = st[9];
    st[9] = st[13];
    st[13] = t1;
    // Row 2 (index 2,6,10,14) rotate left by 2
    int t2 = st[2];
    int t6 = st[6];
    st[2] = st[10];
    st[6] = st[14];
    st[10] = t2;
    st[14] = t6;
    // Row 3 (index 3,7,11,15) rotate left by 3 (right by 1)
    int t3 = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7] = st[3];
    st[3] = t3;
}

void MixColumns(std::vector<int>& st) {
    for (int c = 0; c < 4; ++c) {
        int i0 = c * 4;
        int a0 = st[i0];
        int a1 = st[i0 + 1];
        int a2 = st[i0 + 2];
        int a3 = st[i0 + 3];

        int t0 = xtime(a0) ^ xtime(a1) ^ a1 ^ a2 ^ a3;
        int t1 = a0 ^ xtime(a1) ^ xtime(a2) ^ a2 ^ a3;
        int t2 = a0 ^ a1 ^ xtime(a2) ^ xtime(a3) ^ a3;
        int t3 = xtime(a0) ^ a0 ^ a1 ^ a2 ^ xtime(a3);

        st[i0]     = t0 & 0xFF;
        st[i0 + 1] = t1 & 0xFF;
        st[i0 + 2] = t2 & 0xFF;
        st[i0 + 3] = t3 & 0xFF;
    }
}

void AddRoundKey(std::vector<int>& st, const std::vector<int>& rk, int offset) {
    for (int i = 0; i < 16; ++i) {
        st[i] ^= rk[offset + i];
    }
}

// ----- Key schedule --------------------------------------------------
std::vector<int> KeyExpansion(const std::vector<int>& key) {
    std::vector<int> w(176); // 11 round keys * 16 bytes
    for (int i = 0; i < 16; ++i) w[i] = key[i];

    int bytesGenerated = 16;
    int rconIter = 0;
    while (bytesGenerated < 176) {
        // Temp word
        int t0 = w[bytesGenerated - 4];
        int t1 = w[bytesGenerated - 3];
        int t2 = w[bytesGenerated - 2];
        int t3 = w[bytesGenerated - 1];

        // RotWord + SubWord + Rcon every 16 bytes
        if ((bytesGenerated % 16) == 0) {
            int tmp = t0;
            t0 = sbox[t1];
            t1 = sbox[t2];
            t2 = sbox[t3];
            t3 = sbox[tmp];
            t0 ^= rcon[rconIter];
            ++rconIter;
        }

        // XOR with word 4 positions earlier
        w[bytesGenerated]     = w[bytesGenerated - 16] ^ t0;
        w[bytesGenerated + 1] = w[bytesGenerated - 15] ^ t1;
        w[bytesGenerated + 2] = w[bytesGenerated - 14] ^ t2;
        w[bytesGenerated + 3] = w[bytesGenerated - 13] ^ t3;

        bytesGenerated += 4;
    }
    return w;
}

// ----- Block encryption -----------------------------------------------
void EncryptBlock(std::vector<int>& block, const std::vector<int>& roundKeys) {
    AddRoundKey(block, roundKeys, 0);
    int round = 1;
    while (round < 10) {
        SubBytes(block);
        ShiftRows(block);
        MixColumns(block);
        AddRoundKey(block, roundKeys, round * 16);
        ++round;
    }
    SubBytes(block);
    ShiftRows(block);
    AddRoundKey(block, roundKeys, 160);
}

// ----- Main -----------------------------------------------------------
int main() {
    // Edge‑case plaintexts: all zeros, all 0xFF, and a mixed signed‑extreme vector
    std::vector<int> plainZero(16, 0x00);
    std::vector<int> plainFF(16, 0xFF);

    // Mixed signed‑extreme plaintext (values will be normalized to byte range)
    std::vector<int> rawMixed = {
        -128, -1, 0, 1, 127, -64, 64, -32,
        32, -16, 16, -8, 8, -4, 4, 255
    };
    std::vector<int> plainMixed(16);
    for (int i = 0; i < 16; ++i) {
        plainMixed[i] = rawMixed[i] & 0xFF;
    }

    // Example key: signed‑extreme values (will be normalized)
    std::vector<int> rawKey = {
        -5, -4, -3, -2, -1, 0, 1, 2,
        3, 4, 5, 6, 7, 8, 9, 10
    };
    std::vector<int> key(16);
    for (int i = 0; i < 16; ++i) {
        key[i] = rawKey[i] & 0xFF;
    }

    // Expand key
    std::vector<int> roundKeys = KeyExpansion(key);

    // Encrypt all blocks
    std::vector<int> ctZero = plainZero;
    std::vector<int> ctFF   = plainFF;
    std::vector<int> ctMixed = plainMixed;

    EncryptBlock(ctZero, roundKeys);
    EncryptBlock(ctFF,   roundKeys);
    EncryptBlock(ctMixed, roundKeys);

    // Output results
    std::cout << "Ciphertext (all 0x00 plain): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (ctZero[i] & 0xFF);
    }
    std::cout << std::endl;

    std::cout << "Ciphertext (all 0xFF plain): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (ctFF[i] & 0xFF);
    }
    std::cout << std::endl;

    std::cout << "Ciphertext (mixed signed‑extreme plain): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (ctMixed[i] & 0xFF);
    }
    std::cout << std::endl;

    return 0;
}
