#include <iostream>
#include <iomanip>

/* LLM input variant 2: small-diverse */

// ------------------------------------------------------------
//  AES‑128 core (int only, stack arrays, modular functions)
// ------------------------------------------------------------

class AES128OCB {
public:
    // ----------------------------------------------------------------
    //  Public interface
    // ----------------------------------------------------------------
    void encryptOCB(const int *plain, const int *nonce,
                    const int *key, int *cipher, int *authTag) {
        int roundKeys[44][4];
        expandKey(key, roundKeys);

        // L = AES_K(0^128)   (used for offset derivation)
        int zeroBlk[16] = {0};
        int L[16];
        encryptBlock(zeroBlk, roundKeys, L);

        // Offset = AES_K(NONCE)   (simplified offset for single‑block OCB)
        int offset[16];
        encryptBlock(const_cast<int*>(nonce), roundKeys, offset);

        // tmp = P XOR Offset
        int tmp[16];
        for (int i = 0; i < 16; ++i) {
            tmp[i] = plain[i] ^ offset[i];
        }

        // C* = AES_K(tmp)
        int cStar[16];
        encryptBlock(tmp, roundKeys, cStar);

        // Ciphertext = C* XOR Offset
        for (int i = 0; i < 16; ++i) {
            cipher[i] = cStar[i] ^ offset[i];
        }

        // Tag = AES_K(Offset XOR Ciphertext)
        int tagTmp[16];
        for (int i = 0; i < 16; ++i) {
            tagTmp[i] = offset[i] ^ cipher[i];
        }
        encryptBlock(tagTmp, roundKeys, authTag);
    }

    // ----------------------------------------------------------------
    //  Simple decryption (to verify round‑trip)
    // ----------------------------------------------------------------
    void decryptOCB(const int *cipher, const int *nonce,
                    const int *key, int *plain, const int *authTag) {
        int roundKeys[44][4];
        expandKey(key, roundKeys);

        // Re‑create the same offset
        int offset[16];
        encryptBlock(const_cast<int*>(nonce), roundKeys, offset);

        // C* = Ciphertext XOR Offset
        int cStar[16];
        for (int i = 0; i < 16; ++i) {
            cStar[i] = cipher[i] ^ offset[i];
        }

        // tmp = AES_K^{-1}(C*)
        int tmp[16];
        decryptBlock(cStar, roundKeys, tmp);

        // Plaintext = tmp XOR Offset
        for (int i = 0; i < 16; ++i) {
            plain[i] = tmp[i] ^ offset[i];
        }

        // (Tag verification omitted for brevity)
        (void)authTag; // silence unused‑parameter warning
    }

private:
    // ------------------------------------------------------------
    //  Core AES helpers (all int, no const, stack‑only)
    // ------------------------------------------------------------

    // S‑box (int values 0‑255)
    int sBox[256] = {
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

    // Rcon (int values)
    int rCon[10] = {1,2,4,8,16,32,64,128,27,54};

    // ----------------------------------------------------------------
    //  Key expansion (44 words, each word = 4 bytes)
    // ----------------------------------------------------------------
    void expandKey(const int *key, int w[44][4]) {
        // First 4 words are the cipher key
        for (int i = 0; i < 4; ++i) {
            w[i][0] = key[4*i];
            w[i][1] = key[4*i+1];
            w[i][2] = key[4*i+2];
            w[i][3] = key[4*i+3];
        }

        // Remaining words
        for (int i = 4; i < 44; ++i) {
            int temp[4];
            for (int j = 0; j < 4; ++j) temp[j] = w[i-1][j];

            if (i % 4 == 0) {
                // RotWord
                int t = temp[0];
                temp[0] = temp[1];
                temp[1] = temp[2];
                temp[2] = temp[3];
                temp[3] = t;
                // SubWord
                for (int j = 0; j < 4; ++j) temp[j] = sBox[temp[j]];
                // Rcon
                temp[0] ^= rCon[(i/4)-1];
            }

            for (int j = 0; j < 4; ++j) {
                w[i][j] = w[i-4][j] ^ temp[j];
            }
        }
    }

    // ----------------------------------------------------------------
    //  One round of SubBytes
    // ----------------------------------------------------------------
    void subBytes(int *state) {
        for (int i = 0; i < 16; ++i) {
            state[i] = sBox[state[i]];
        }
    }

    // ----------------------------------------------------------------
    //  ShiftRows (in‑place)
    // ----------------------------------------------------------------
    void shiftRows(int *st) {
        int tmp[16];
        // Row 0 (no shift)
        tmp[0]  = st[0];   tmp[4]  = st[4];   tmp[8]  = st[8];   tmp[12] = st[12];
        // Row 1 (shift left 1)
        tmp[1]  = st[5];   tmp[5]  = st[9];   tmp[9]  = st[13];  tmp[13] = st[1];
        // Row 2 (shift left 2)
        tmp[2]  = st[10];  tmp[6]  = st[14];  tmp[10] = st[2];   tmp[14] = st[6];
        // Row 3 (shift left 3)
        tmp[3]  = st[15];  tmp[7]  = st[3];   tmp[11] = st[7];   tmp[15] = st[11];
        for (int i = 0; i < 16; ++i) st[i] = tmp[i];
    }

    // ----------------------------------------------------------------
    //  MixColumns (in‑place, using xtime)
    // ----------------------------------------------------------------
    int xtime(int v) {
        int r = (v << 1) & 0xFF;
        if (v & 0x80) r ^= 0x1B;
        return r;
    }

    void mixColumns(int *st) {
        for (int c = 0; c < 4; ++c) {
            int i0 = st[4*c];
            int i1 = st[4*c+1];
            int i2 = st[4*c+2];
            int i3 = st[4*c+3];

            int a = xtime(i0) ^ xtime(i1) ^ i1 ^ i2 ^ i3;
            int b = i0 ^ xtime(i1) ^ xtime(i2) ^ i2 ^ i3;
            int d = i0 ^ i1 ^ xtime(i2) ^ xtime(i3) ^ i3;
            int e = xtime(i0) ^ i0 ^ i1 ^ i2 ^ xtime(i3);

            st[4*c]   = a;
            st[4*c+1] = b;
            st[4*c+2] = d;
            st[4*c+3] = e;
        }
    }

    // ----------------------------------------------------------------
    //  AddRoundKey (in‑place)
    // ----------------------------------------------------------------
    void addRoundKey(int *st, const int w[44][4], int round) {
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                st[4*col + row] ^= w[round*4 + col][row];
            }
        }
    }

    // ----------------------------------------------------------------
    //  Encrypt one 16‑byte block
    // ----------------------------------------------------------------
    void encryptBlock(const int *inp, const int w[44][4], int *out) {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = inp[i];

        addRoundKey(state, w, 0);

        for (int r = 1; r <= 9; ++r) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, w, r);
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, w, 10);

        for (int i = 0; i < 16; ++i) out[i] = state[i];
    }

    // ----------------------------------------------------------------
    //  Decrypt one 16‑byte block (inverse operations)
    // ----------------------------------------------------------------
    void invSubBytes(int *st) {
        // Build inverse S‑box on the fly (small table)
        static int invS[256] = {0};
        static bool built = false;
        if (!built) {
            for (int i = 0; i < 256; ++i) invS[sBox[i]] = i;
            built = true;
        }
        for (int i = 0; i < 16; ++i) st[i] = invS[st[i]];
    }

    void invShiftRows(int *st) {
        int tmp[16];
        // Row 0
        tmp[0]  = st[0];   tmp[4]  = st[4];   tmp[8]  = st[8];   tmp[12] = st[12];
        // Row 1 (right shift 1)
        tmp[1]  = st[13];  tmp[5]  = st[1];   tmp[9]  = st[5];   tmp[13] = st[9];
        // Row 2 (right shift 2)
        tmp[2]  = st[10];  tmp[6]  = st[14];  tmp[10] = st[2];   tmp[14] = st[6];
        // Row 3 (right shift 3)
        tmp[3]  = st[7];   tmp[7]  = st[11];  tmp[11] = st[15];  tmp[15] = st[3];
        for (int i = 0; i < 16; ++i) st[i] = tmp[i];
    }

    void invMixColumns(int *st) {
        for (int c = 0; c < 4; ++c) {
            int a0 = st[4*c];
            int a1 = st[4*c+1];
            int a2 = st[4*c+2];
            int a3 = st[4*c+3];

            int u = xtime(xtime(a0 ^ a2));
            int v = xtime(xtime(a1 ^ a3));

            int a0n = a0 ^ u;
            int a1n = a1 ^ v;
            int a2n = a2 ^ u;
            int a3n = a3 ^ v;

            // Now apply normal MixColumns on the transformed column
            int b0 = xtime(a0n) ^ xtime(a1n) ^ a1n ^ a2n ^ a3n;
            int b1 = a0n ^ xtime(a1n) ^ xtime(a2n) ^ a2n ^ a3n;
            int b2 = a0n ^ a1n ^ xtime(a2n) ^ xtime(a3n) ^ a3n;
            int b3 = xtime(a0n) ^ a0n ^ a1n ^ a2n ^ xtime(a3n);

            st[4*c]   = b0;
            st[4*c+1] = b1;
            st[4*c+2] = b2;
            st[4*c+3] = b3;
        }
    }

    void decryptBlock(const int *inp, const int w[44][4], int *out) {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = inp[i];

        addRoundKey(state, w, 10);
        invShiftRows(state);
        invSubBytes(state);

        for (int r = 9; r >= 1; --r) {
            addRoundKey(state, w, r);
            invMixColumns(state);
            invShiftRows(state);
            invSubBytes(state);
        }

        addRoundKey(state, w, 0);
        for (int i = 0; i < 16; ++i) out[i] = state[i];
    }
};

// ------------------------------------------------------------
//  Demo / driver (no cin, internal data, prints results)
// ------------------------------------------------------------
int main() {
    // 128‑bit key (16 bytes) – varied pattern
    int key[16] = {
        0x00,0x11,0x22,0x33,
        0x44,0x55,0x66,0x77,
        0x88,0x99,0xaa,0xbb,
        0xcc,0xdd,0xee,0xff
    };

    // 16‑byte nonce – reverse sequence
    int nonce[16] = {
        0xff,0xee,0xdd,0xcc,
        0xbb,0xaa,0x99,0x88,
        0x77,0x66,0x55,0x44,
        0x33,0x22,0x11,0x00
    };

    // Plaintext – alternating low/high values
    int plain[16] = {
        0x00,0xff,0x01,0xfe,
        0x02,0xfd,0x03,0xfc,
        0x04,0xfb,0x05,0xfa,
        0x06,0xf9,0x07,0xf8
    };

    int cipher[16];
    int tag[16];
    int recovered[16];

    AES128OCB engine;
    engine.encryptOCB(plain, nonce, key, cipher, tag);
    engine.decryptOCB(cipher, nonce, key, recovered, tag);

    // Print all data
    std::cout << "Plaintext : ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i] << ' ';
    std::cout << "\nCiphertext: ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << cipher[i] << ' ';
    std::cout << "\nTag       : ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << tag[i] << ' ';
    std::cout << "\nRecovered : ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << recovered[i] << ' ';
    std::cout << std::dec << std::endl;

    return 0;
}
