#include <iostream>
#include <vector>
#include <iomanip>
/* LLM input variant 9: medium-deterministic-random */

int SBOX[256] = {
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

int RCON[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

int mul2(int b) {
    int res = b << 1;
    if (b & 0x80) res ^= 0x1b;
    return res & 0xff;
}
int mul3(int b) {
    return mul2(b) ^ b;
}

class AES128CFB {
public:
    std::vector<int> roundKey;   // 176 bytes = 11 round keys
    AES128CFB(const std::vector<int>& key) {
        keyExpansion(key);
    }

    void encryptCFB(const std::vector<int>& iv,
                    const std::vector<int>& plain,
                    std::vector<int>& cipher) {
        std::vector<int> feedback = iv;
        int blocks = (int)plain.size() / 16;
        for (int i = 0; i < blocks; ++i) {
            std::vector<int> encBlock(16);
            encryptBlock(feedback, encBlock);
            for (int j = 0; j < 16; ++j) {
                int c = encBlock[j] ^ plain[i*16 + j];
                cipher.push_back(c & 0xff);
                feedback[j] = c & 0xff;   // next feedback is ciphertext
            }
        }
    }

    void decryptCFB(const std::vector<int>& iv,
                    const std::vector<int>& cipher,
                    std::vector<int>& plain) {
        std::vector<int> feedback = iv;
        int blocks = (int)cipher.size() / 16;
        for (int i = 0; i < blocks; ++i) {
            std::vector<int> encBlock(16);
            encryptBlock(feedback, encBlock);
            for (int j = 0; j < 16; ++j) {
                int p = encBlock[j] ^ cipher[i*16 + j];
                plain.push_back(p & 0xff);
                feedback[j] = cipher[i*16 + j]; // feedback is ciphertext
            }
        }
    }

private:
    void keyExpansion(const std::vector<int>& key) {
        roundKey.resize(176);
        for (int i = 0; i < 16; ++i) roundKey[i] = key[i] & 0xff;
        int bytes = 16;
        int rconIter = 1;
        while (bytes < 176) {
            int temp0 = roundKey[bytes-4];
            int temp1 = roundKey[bytes-3];
            int temp2 = roundKey[bytes-2];
            int temp3 = roundKey[bytes-1];
            if ((bytes % 16) == 0) {
                // RotWord
                int t = temp0;
                temp0 = temp1;
                temp1 = temp2;
                temp2 = temp3;
                temp3 = t;
                // SubWord
                temp0 = SBOX[temp0];
                temp1 = SBOX[temp1];
                temp2 = SBOX[temp2];
                temp3 = SBOX[temp3];
                // Rcon
                temp0 ^= RCON[rconIter];
                ++rconIter;
            }
            roundKey[bytes] = (roundKey[bytes-16] ^ temp0) & 0xff;
            roundKey[bytes+1] = (roundKey[bytes-15] ^ temp1) & 0xff;
            roundKey[bytes+2] = (roundKey[bytes-14] ^ temp2) & 0xff;
            roundKey[bytes+3] = (roundKey[bytes-13] ^ temp3) & 0xff;
            bytes += 4;
        }
    }

    void addRoundKey(std::vector<int>& state, int round) {
        for (int i = 0; i < 16; ++i) {
            state[i] ^= roundKey[round*16 + i];
        }
    }

    void subBytes(std::vector<int>& state) {
        for (int i = 0; i < 16; ++i) state[i] = SBOX[state[i]];
    }

    void shiftRows(std::vector<int>& s) {
        int tmp;
        // Row 1
        tmp = s[1]; s[1] = s[5]; s[5] = s[9]; s[9] = s[13]; s[13] = tmp;
        // Row 2
        tmp = s[2]; s[2] = s[10]; s[10] = tmp;
        tmp = s[6]; s[6] = s[14]; s[14] = tmp;
        // Row 3
        tmp = s[3]; s[3] = s[15]; s[15] = s[11]; s[11] = s[7]; s[7] = tmp;
    }

    void mixColumns(std::vector<int>& s) {
        for (int c = 0; c < 4; ++c) {
            int i0 = c*4;
            int a0 = s[i0];
            int a1 = s[i0+1];
            int a2 = s[i0+2];
            int a3 = s[i0+3];
            int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            s[i0]   = r0 & 0xff;
            s[i0+1] = r1 & 0xff;
            s[i0+2] = r2 & 0xff;
            s[i0+3] = r3 & 0xff;
        }
    }

    void encryptBlock(const std::vector<int>& in, std::vector<int>& out) {
        std::vector<int> state = in;
        addRoundKey(state, 0);
        for (int round = 1; round <= 9; ++round) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
        }
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 10);
        out = state;
    }
};

void printHex(const std::vector<int>& data, const char* title) {
    std::cout << title << "\n";
    for (int i = 0; i < (int)data.size(); ++i) {
        if (i % 16 == 0) std::cout << "\n";
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (data[i] & 0xff) << " ";
    }
    std::cout << std::dec << "\n\n";
}

int main() {
    // deterministic 128‑bit key
    std::vector<int> key(16);
    for (int i = 0; i < 16; ++i) key[i] = ((i * 37) + 0x3C) & 0xff;

    // deterministic IV
    std::vector<int> iv(16);
    for (int i = 0; i < 16; ++i) iv[i] = ((i * 53) + 0xA7) & 0xff;

    // deterministic medium-size plaintext (64 bytes = 4 blocks)
    std::vector<int> plain;
    for (int i = 0; i < 64; ++i) plain.push_back(((i * 61) + 0x5F) & 0xff);

    AES128CFB aes(key);

    std::vector<int> cipher;
    aes.encryptCFB(iv, plain, cipher);

    std::vector<int> recovered;
    aes.decryptCFB(iv, cipher, recovered);

    printHex(key, "Key:");
    printHex(iv, "IV:");
    printHex(plain, "Plaintext:");
    printHex(cipher, "Ciphertext:");
    printHex(recovered, "Recovered Plaintext:");

    // verify correctness
    bool ok = true;
    for (int i = 0; i < (int)plain.size(); ++i) {
        if ((plain[i] & 0xff) != (recovered[i] & 0xff)) { ok = false; break; }
    }
    std::cout << (ok ? "Success: recovered matches original." : "Error: mismatch.") << std::endl;
    return 0;
}
