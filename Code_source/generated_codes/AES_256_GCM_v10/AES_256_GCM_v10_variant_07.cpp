/* LLM input variant 7: reverse-adversarial */
// AES‑256‑GCM implementation – version #10
// Only int and float types are used (no double, long, unsigned, const)
// Stack‑allocated arrays, class‑based design, branch‑minimized where feasible

#include <iostream>
#include <iomanip>
#include <cstring>

class Aes256Gcm {
public:
    // --------------------------------------------------------------------
    // Public interface
    // --------------------------------------------------------------------
    void encrypt(
        int *plain, int plainLen,
        int *aad,   int aadLen,
        int *nonce, int nonceLen,
        int *key,   int keyLen,          // keyLen must be 32 (256‑bit)
        int *outCipher, int &outTag)   // tag is 16‑byte (128‑bit) value
    {
        // 1. Derive round keys
        expandKey(key);

        // 2. Compute hash sub‑key H = AES_K(0^128)
        int zeroBlock[16];
        for (int i = 0; i < 16; ++i) zeroBlock[i] = 0;
        int H[16];
        encryptBlock(zeroBlock, H);

        // 3. Prepare initial counter J0 = nonce || 0x00000001
        int J0[16];
        for (int i = 0; i < 12; ++i) J0[i] = nonce[i];
        J0[12] = 0; J0[13] = 0; J0[14] = 0; J0[15] = 1;

        // 4. Encrypt plaintext (counter mode)
        int ctr[16];
        for (int i = 0; i < 16; ++i) ctr[i] = J0[i];
        int pos = 0;
        while (pos < plainLen) {
            int stream[16];
            encryptBlock(ctr, stream);
            // XOR plaintext with keystream
            for (int i = 0; i < 16 && pos + i < plainLen; ++i) {
                outCipher[pos + i] = plain[pos + i] ^ stream[i];
            }
            // Increment counter (big‑endian)
            int carry = 1;
            for (int i = 15; i >= 12; --i) {
                int sum = ctr[i] + carry;
                ctr[i] = sum & 0xFF;
                carry = (sum >> 8) & 0x1;
            }
            pos += 16;
        }

        // 5. Compute GHASH over AAD || ciphertext
        int S[16];
        for (int i = 0; i < 16; ++i) S[i] = 0;
        // AAD
        ghashUpdate(aad,   aadLen,   H, S);
        // Ciphertext
        ghashUpdate(outCipher, plainLen, H, S);
        // Length block
        int lenBlock[16];
        for (int i = 0; i < 16; ++i) lenBlock[i] = 0;
        // Lengths are in bits; store as 64‑bit high || 64‑bit low
        int aadBits = aadLen * 8;
        int ctBits  = plainLen * 8;
        // high 64 bits = AAD length
        for (int i = 0; i < 8; ++i) lenBlock[i] = (aadBits >> ((7 - i) * 8)) & 0xFF;
        // low 64 bits = ciphertext length
        for (int i = 0; i < 8; ++i) lenBlock[8 + i] = (ctBits >> ((7 - i) * 8)) & 0xFF;
        ghashUpdate(lenBlock, 16, H, S);

        // 6. Tag = AES_K(J0) XOR S
        int encJ0[16];
        encryptBlock(J0, encJ0);
        for (int i = 0; i < 16; ++i) outTag = (outTag << 8) | (encJ0[i] ^ S[i] & 0xFF);

        // Tag is returned as a 128‑bit integer packed into an int (only low 32 bits are usable)
        // For demonstration we keep the full 16‑byte array in outTag (packed as 32‑bit int)
    }

    // --------------------------------------------------------------------
    // Helper to print a byte array as hex
    // --------------------------------------------------------------------
    void printHex(const char *label, int *data, int len) {
        std::cout << label;
        for (int i = 0; i < len; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (data[i] & 0xFF);
        }
        std::cout << std::dec << std::endl;
    }

private:
    // --------------------------------------------------------------------
    // AES‑256 key schedule (14 rounds + initial)
    // --------------------------------------------------------------------
    int roundKey[15][4][4]; // 15 round keys, each 4x4 byte matrix

    void expandKey(int *key) {
        // First round key = first 16 bytes of key
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                roundKey[0][i][j] = key[i * 4 + j];
        // Second round key = last 16 bytes of key
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                roundKey[1][i][j] = key[16 + i * 4 + j];

        // Rcon values (no const, generated on‑the‑fly)
        int rcon = 1;
        for (int i = 2; i < 15; ++i) {
            // Temp word = last column of previous round key
            int temp[4];
            for (int t = 0; t < 4; ++t) temp[t] = roundKey[i - 1][t][3];
            // RotWord
            int rot = temp[0];
            for (int t = 0; t < 3; ++t) temp[t] = temp[t + 1];
            temp[3] = rot;
            // SubWord via S‑box
            for (int t = 0; t < 4; ++t) temp[t] = sbox(temp[t] & 0xFF);
            // XOR with Rcon on first byte
            temp[0] ^= rcon;
            // Update Rcon (multiply by 2 in GF(2^8))
            rcon = (rcon << 1) ^ ((rcon >> 7) * 0x11B);
            // Generate columns 0‑3
            for (int col = 0; col < 4; ++col) {
                for (int row = 0; row < 4; ++row) {
                    int prev = roundKey[i - 2][row][col];
                    int add  = (col == 0) ? temp[row] : roundKey[i][row][col - 1];
                    roundKey[i][row][col] = prev ^ add;
                }
            }
        }
    }

    // --------------------------------------------------------------------
    // Single‑block AES encryption (16‑byte input → 16‑byte output)
    // --------------------------------------------------------------------
    void encryptBlock(int *in, int *out) {
        int state[4][4];
        // Copy input to state (column‑major)
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                state[j][i] = in[i * 4 + j] & 0xFF;

        // Initial AddRoundKey
        addRoundKey(state, roundKey[0]);

        // 13 main rounds
        for (int r = 1; r < 14; ++r) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, roundKey[r]);
        }

        // Final round (no MixColumns)
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, roundKey[14]);

        // Write state to output (column‑major)
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                out[i * 4 + j] = state[j][i] & 0xFF;
    }

    // --------------------------------------------------------------------
    // AES core transformations (branch‑minimized where possible)
    // --------------------------------------------------------------------
    void subBytes(int s[4][4]) {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                s[i][j] = sbox(s[i][j] & 0xFF);
    }

    void shiftRows(int s[4][4]) {
        // Row 1: shift left by 1
        int t = s[1][0];
        s[1][0] = s[1][1]; s[1][1] = s[1][2]; s[1][2] = s[1][3]; s[1][3] = t;
        // Row 2: shift left by 2 (swap pairs)
        int u = s[2][0];
        s[2][0] = s[2][2]; s[2][2] = u;
        u = s[2][1];
        s[2][1] = s[2][3]; s[2][3] = u;
        // Row 3: shift left by 3 (right by 1)
        t = s[3][3];
        s[3][3] = s[3][2]; s[3][2] = s[3][1]; s[3][1] = s[3][0]; s[3][0] = t;
    }

    void mixColumns(int s[4][4]) {
        for (int c = 0; c < 4; ++c) {
            int a0 = s[0][c], a1 = s[1][c], a2 = s[2][c], a3 = s[3][c];
            int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            s[0][c] = r0 & 0xFF; s[1][c] = r1 & 0xFF;
            s[2][c] = r2 & 0xFF; s[3][c] = r3 & 0xFF;
        }
    }

    void addRoundKey(int s[4][4], int rk[4][4]) {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                s[i][j] ^= rk[i][j];
    }

    // --------------------------------------------------------------------
    // Galois field helpers for GHASH
    // --------------------------------------------------------------------
    void ghashUpdate(int *data, int dataLen, int *H, int *S) {
        int blocks = (dataLen + 15) / 16;
        int offset = 0;
        while (blocks--) {
            int block[16];
            for (int i = 0; i < 16; ++i) {
                int srcIdx = offset + i;
                block[i] = (srcIdx < dataLen) ? (data[srcIdx] & 0xFF) : 0;
            }
            // S = (S XOR block) * H
            for (int i = 0; i < 16; ++i) S[i] ^= block[i];
            multiplyGF128(S, H, S);
            offset += 16;
        }
    }

    // Multiply two 128‑bit numbers in GF(2^128) (bit‑wise, no branches)
    void multiplyGF128(int *X, int *Y, int *Z) {
        int V[16];
        for (int i = 0; i < 16; ++i) V[i] = Y[i];
        for (int i = 0; i < 16; ++i) Z[i] = 0;
        for (int i = 0; i < 128; ++i) {
            int bit = (X[i / 8] >> (7 - (i % 8))) & 1;
            for (int j = 0; j < 16; ++j) Z[j] ^= V[j] * bit;
            // Shift V right by 1
            int lsb = V[15] & 1;
            for (int j = 15; j > 0; --j) V[j] = (V[j] >> 1) | ((V[j - 1] & 1) << 7);
            V[0] >>= 1;
            // Conditional reduction (no branch: use mask)
            int mask = -lsb; // all 1s if lsb==1 else 0
            int R[16] = {0xE1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            for (int j = 0; j < 16; ++j) V[j] ^= R[j] & mask;
        }
    }

    // --------------------------------------------------------------------
    // Byte‑level arithmetic helpers
    // --------------------------------------------------------------------
    int mul2(int x) {
        int shifted = (x << 1) & 0xFF;
        int overflow = (x >> 7) & 1;
        return shifted ^ (overflow * 0x1B);
    }

    int mul3(int x) { return mul2(x) ^ x; }

    // AES S‑box (hard‑coded, no const)
    int sbox(int in) {
        static int table[256] = {
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
        return table[in & 0xFF];
    }
};

// ------------------------------------------------------------------------
// Main – generate edge‑case inputs, run encryption, print results
// ------------------------------------------------------------------------
int main() {
    // Edge‑case 1: zero‑length plaintext & AAD
    int key[32];
    for (int i = 0; i < 32; ++i) key[i] = 255 - i;          // descending deterministic key
    int nonce[12];
    for (int i = 0; i < 12; ++i) nonce[i] = 0xFF - i;      // descending IV pattern
    int plain1[1] = {};                                   // empty
    int aad1[1]   = {};                                   // empty
    int cipher1[1];
    int tag1 = 0;
    Aes256Gcm gcm1;
    gcm1.encrypt(plain1, 0, aad1, 0, nonce, 12, key, 32, cipher1, tag1);
    gcm1.printHex("Cipher (empty): ", cipher1, 0);
    std::cout << "Tag (empty)   : " << std::hex << std::setw(8) << std::setfill('0')
              << (tag1 & 0xFFFFFFFF) << std::dec << std::endl;

    // Edge‑case 2: reverse‑ordered maximal short plaintext (128 bytes) with non‑empty AAD (48 bytes)
    int plain2[128];
    for (int i = 0; i < 128; ++i) plain2[i] = (255 - i) & 0xFF;   // descending bytes
    int aad2[48];
    for (int i = 0; i < 48; ++i) aad2[i] = (239 - i) & 0xFF;    // descending AAD
    int cipher2[128];
    int tag2 = 0;
    Aes256Gcm gcm2;
    gcm2.encrypt(plain2, 128, aad2, 48, nonce, 12, key, 32, cipher2, tag2);
    gcm2.printHex("Cipher (128B): ", cipher2, 128);
    std::cout << "Tag (128B)     : " << std::hex << std::setw(8) << std::setfill('0')
              << (tag2 & 0xFFFFFFFF) << std::dec << std::endl;

    return 0;
}
