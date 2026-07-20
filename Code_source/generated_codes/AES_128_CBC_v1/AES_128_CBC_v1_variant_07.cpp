#include <iostream>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */

// ------------------------------------------------------------
// AES-128 in CBC mode – class based, heap allocation, verbose
// ------------------------------------------------------------

class AES128_CBC {
public:
    // Constructor: receives a 16‑byte key and a 16‑byte IV
    AES128_CBC(int* keyBytes, int* ivBytes) {
        // allocate space for 11 round keys (each 16 bytes)
        roundKey = new int[176];           // 11 * 16
        // allocate space for the chaining vector
        ivBlock = new int[16];
        // copy IV into internal buffer
        int idx = 0;
        while (idx < 16) {
            ivBlock[idx] = ivBytes[idx];
            idx = idx + 1;
        }
        // expand the key
        keyExpansion(keyBytes);
    }

    // Destructor: free heap memory
    ~AES128_CBC() {
        delete[] roundKey;
        delete[] ivBlock;
    }

    // Encrypt a plaintext of 'len' bytes (must be multiple of 16)
    void encrypt(int* plain, int len, int* cipher) {
        int blockIdx = 0;
        while (blockIdx < len) {
            // ----- CBC step : XOR with previous cipher (or IV) -----
            int xorIdx = 0;
            while (xorIdx < 16) {
                plain[blockIdx + xorIdx] = plain[blockIdx + xorIdx] ^ ivBlock[xorIdx];
                xorIdx = xorIdx + 1;
            }

            // ----- Encrypt the block -----
            encryptOneBlock(&plain[blockIdx]);

            // ----- Copy ciphertext to output and update IV -----
            int outIdx = 0;
            while (outIdx < 16) {
                cipher[blockIdx + outIdx] = plain[blockIdx + outIdx];
                ivBlock[outIdx] = cipher[blockIdx + outIdx];
                outIdx = outIdx + 1;
            }

            blockIdx = blockIdx + 16;
        }
    }

private:
    int* roundKey;   // 176 bytes = 11 * 16
    int* ivBlock;    // 16 bytes

    // --------------------------------------------------------
    // Key expansion – produces 11 round keys (each 16 bytes)
    // --------------------------------------------------------
    void keyExpansion(int* key) {
        // copy original key as first round key
        int i = 0;
        while (i < 16) {
            roundKey[i] = key[i];
            i = i + 1;
        }

        // temporary word
        int temp[4];

        // generate remaining words
        int wordIdx = 4; // start from the 5th word
        while (wordIdx < 44) {
            // copy previous word into temp
            int t = 0;
            while (t < 4) {
                temp[t] = roundKey[(wordIdx - 1) * 4 + t];
                t = t + 1;
            }

            // if wordIdx is multiple of 4, apply core transformation
            if ((wordIdx % 4) == 0) {
                // rotate left
                int rot = temp[0];
                temp[0] = temp[1];
                temp[1] = temp[2];
                temp[2] = temp[3];
                temp[3] = rot;

                // subBytes on each byte
                int s = 0;
                while (s < 4) {
                    temp[s] = sBox[temp[s]];
                    s = s + 1;
                }

                // XOR first byte with Rcon
                temp[0] = temp[0] ^ rCon[wordIdx / 4];
            }

            // XOR with word 4 positions earlier
            int j = 0;
            while (j < 4) {
                roundKey[wordIdx * 4 + j] = roundKey[(wordIdx - 4) * 4 + j] ^ temp[j];
                j = j + 1;
            }

            wordIdx = wordIdx + 1;
        }
    }

    // --------------------------------------------------------
    // Encrypt a single 16‑byte block (in‑place)
    // --------------------------------------------------------
    void encryptOneBlock(int* block) {
        // initial AddRoundKey
        addRoundKey(block, 0);

        int round = 1;
        while (round <= 9) {
            subBytes(block);
            shiftRows(block);
            mixColumns(block);
            addRoundKey(block, round);
            round = round + 1;
        }

        // final round (no MixColumns)
        subBytes(block);
        shiftRows(block);
        addRoundKey(block, 10);
    }

    // --------------------------------------------------------
    // Helper transformations
    // --------------------------------------------------------
    void addRoundKey(int* state, int round) {
        int i = 0;
        while (i < 16) {
            state[i] = state[i] ^ roundKey[round * 16 + i];
            i = i + 1;
        }
    }

    void subBytes(int* state) {
        int i = 0;
        while (i < 16) {
            state[i] = sBox[state[i]];
            i = i + 1;
        }
    }

    void shiftRows(int* state) {
        // Row 1 : shift left by 1
        int tmp = state[1];
        state[1]  = state[5];
        state[5]  = state[9];
        state[9]  = state[13];
        state[13] = tmp;

        // Row 2 : shift left by 2
        int tmp1 = state[2];
        int tmp2 = state[6];
        state[2]  = state[10];
        state[6]  = state[14];
        state[10] = tmp1;
        state[14] = tmp2;

        // Row 3 : shift left by 3 (right by 1)
        int tmp3 = state[15];
        state[15] = state[11];
        state[11] = state[7];
        state[7]  = state[3];
        state[3]  = tmp3;
    }

    // Multiply by 2 in GF(2^8)
    int xtime(int x) {
        int shifted = x << 1;
        if ((x & 0x80) != 0) {
            shifted = shifted ^ 0x1b;
        }
        return shifted & 0xff;
    }

    void mixColumns(int* state) {
        int col = 0;
        while (col < 4) {
            int idx0 = col * 4 + 0;
            int idx1 = col * 4 + 1;
            int idx2 = col * 4 + 2;
            int idx3 = col * 4 + 3;

            int a0 = state[idx0];
            int a1 = state[idx1];
            int a2 = state[idx2];
            int a3 = state[idx3];

            int b0 = xtime(a0) ^ xtime(a1) ^ a1 ^ a2 ^ a3;
            int b1 = a0 ^ xtime(a1) ^ xtime(a2) ^ a2 ^ a3;
            int b2 = a0 ^ a1 ^ xtime(a2) ^ xtime(a3) ^ a3;
            int b3 = xtime(a0) ^ a0 ^ a1 ^ a2 ^ xtime(a3);

            state[idx0] = b0;
            state[idx1] = b1;
            state[idx2] = b2;
            state[idx3] = b3;

            col = col + 1;
        }
    }

    // --------------------------------------------------------
    // Static tables (initialized directly, no const)
    // --------------------------------------------------------
    static int sBox[256];
    static int rCon[11];
};

// -----------------------------------------------------------------
// S‑Box definition (AES standard)
// -----------------------------------------------------------------
int AES128_CBC::sBox[256] = {
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

int AES128_CBC::rCon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

// ------------------------------------------------------------
// Demo in main: encrypt multiple blocks with reversed adversarial data
// ------------------------------------------------------------
int main() {
    // ---- 16‑byte key (reverse order) ----
    int* key = new int[16];
    int k = 0;
    while (k < 16) {
        key[k] = 15 - k;                 // 15,14,...,0
        k = k + 1;
    }

    // ---- 16‑byte IV (reverse order) ----
    int* iv = new int[16];
    int v = 0;
    while (v < 16) {
        iv[v] = 0xAF - v;                // 0xAF,0xAE,...,0xA0
        v = v + 1;
    }

    // ---- Plaintext: 64 bytes (four blocks) ----
    int* plain = new int[64];
    int p = 0;
    while (p < 64) {
        plain[p] = (0xFF - (p * 5)) & 0xFF;  // descending pattern with step
        p = p + 1;
    }

    // Allocate output buffer
    int* cipher = new int[64];

    // Create AES object and encrypt
    AES128_CBC aes(key, iv);
    aes.encrypt(plain, 64, cipher);

    // Print ciphertext as hex
    std::cout << "Ciphertext (hex):" << std::endl;
    int i = 0;
    while (i < 64) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xFF);
        if ((i + 1) % 16 == 0) std::cout << std::endl;
        else std::cout << " ";
        i = i + 1;
    }

    // Cleanup
    delete[] key;
    delete[] iv;
    delete[] plain;
    delete[] cipher;

    return 0;
}
