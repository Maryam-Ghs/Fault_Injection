#include <iostream>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */

// ------------------------------------------------------------
//  Simple ARIA‑128 implementation (branch‑minimized, stack‑arrays)
// ------------------------------------------------------------

// ------------------------------------------------------------------
//  GF(2^8) multiplication (modulo x^8+x^4+x^3+x+1 = 0x11b)
// ------------------------------------------------------------------
int gfMul(int a, int b) {
    int p = 0;
    int i = 0;
    for (i = 0; i < 8; ++i) {
        p ^= (b & 1) * a;
        int hi = a & 0x80;
        a = (a << 1) & 0xFF;
        a ^= (hi >> 7) * 0x1B;
        b >>= 1;
    }
    return p & 0xFF;
}

// ------------------------------------------------------------------
//  S‑boxes (S1 = AES S‑box, S2 = its inverse)
// ------------------------------------------------------------------
static int S1[256];
static int S2[256];

// ------------------------------------------------------------------
//  Initialise the two substitution tables (no const, filled once)
// ------------------------------------------------------------------
void initSboxes() {
    // AES S‑box values (hard‑coded, signed int)
    int raw[256] = {
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
    int i;
    for (i = 0; i < 256; ++i) {
        S1[i] = raw[i] & 0xFF;
    }
    // Build inverse table S2
    for (i = 0; i < 256; ++i) {
        S2[S1[i]] = i;
    }
}

// ------------------------------------------------------------------
//  Substitution layer (choice of S‑box passed as pointer)
// ------------------------------------------------------------------
void subBytes(const int in[16], int out[16], const int *sb) {
    int i;
    for (i = 0; i < 16; ++i) {
        out[i] = sb[in[i] & 0xFF] & 0xFF;
    }
}

// ------------------------------------------------------------------
//  Linear diffusion M  (constants 0x0e,0x0b,0x0d,0x09)
// ------------------------------------------------------------------
void diffM(const int in[16], int out[16]) {
    int col, i0, i1, i2, i3;
    for (col = 0; col < 4; ++col) {
        i0 = in[col];
        i1 = in[4 + col];
        i2 = in[8 + col];
        i3 = in[12 + col];
        out[col]      = gfMul(0x0e, i0) ^ gfMul(0x0b, i1) ^ gfMul(0x0d, i2) ^ gfMul(0x09, i3);
        out[4 + col]  = gfMul(0x09, i0) ^ gfMul(0x0e, i1) ^ gfMul(0x0b, i2) ^ gfMul(0x0d, i3);
        out[8 + col]  = gfMul(0x0d, i0) ^ gfMul(0x09, i1) ^ gfMul(0x0e, i2) ^ gfMul(0x0b, i3);
        out[12 + col] = gfMul(0x0b, i0) ^ gfMul(0x0d, i1) ^ gfMul(0x09, i2) ^ gfMul(0x0e, i3);
    }
}

// ------------------------------------------------------------------
//  Linear diffusion M' (constants 0x02,0x03,0x01,0x01)
// ------------------------------------------------------------------
void diffMprime(const int in[16], int out[16]) {
    int col, a0, a1, a2, a3;
    for (col = 0; col < 4; ++col) {
        a0 = in[col];
        a1 = in[4 + col];
        a2 = in[8 + col];
        a3 = in[12 + col];
        out[col]      = gfMul(0x02, a0) ^ gfMul(0x03, a1) ^ a2 ^ a3;
        out[4 + col]  = a0 ^ gfMul(0x02, a1) ^ gfMul(0x03, a2) ^ a3;
        out[8 + col]  = a0 ^ a1 ^ gfMul(0x02, a2) ^ gfMul(0x03, a3);
        out[12 + col] = gfMul(0x03, a0) ^ a1 ^ a2 ^ gfMul(0x02, a3);
    }
}

// ------------------------------------------------------------------
//  XOR with round key (in‑place)
// ------------------------------------------------------------------
void addRoundKey(int state[16], const int rk[16]) {
    int i;
    for (i = 0; i < 16; ++i) {
        state[i] ^= rk[i];
    }
}

// ------------------------------------------------------------------
//  Rotate a 128‑bit array left by n bytes (n ∈ {0,4,8,12})
// ------------------------------------------------------------------
void rotBytes(const int in[16], int out[16], int n) {
    int i;
    for (i = 0; i < 16; ++i) {
        out[i] = in[(i + n) & 0x0F];
    }
}

// ------------------------------------------------------------------
//  Generate 13 round keys for ARIA‑128 (stack‑based)
// ------------------------------------------------------------------
void keySchedule(const int master[16], int roundKey[13][16]) {
    static int CK1[16] = {0x51,0x7c,0xc1,0xb7,0x27,0x22,0x0a,0x94,0xfe,0x13,0xab,0xe8,0xfa,0x9a,0x6e,0xe0};
    static int CK2[16] = {0x6d,0xb1,0x4a,0xcc,0x9e,0x21,0xc8,0x20,0xff,0x28,0xb1,0xd5,0xef,0x5d,0xe2,0xb0};
    static int CK3[16] = {0xdb,0x92,0x37,0x1d,0x21,0x26,0xe9,0x70,0x03,0x24,0x97,0x75,0x04,0xe8,0xc9,0x0e};

    int tmp[16], wk[4][16];
    int i, j;

    for (i = 0; i < 16; ++i) wk[0][i] = master[i] & 0xFF;

    for (i = 0; i < 16; ++i) tmp[i] = wk[0][i] ^ CK1[i];
    subBytes(tmp, wk[1], S1);
    diffM(wk[1], wk[1]);

    for (i = 0; i < 16; ++i) tmp[i] = wk[1][i] ^ CK2[i];
    subBytes(tmp, wk[2], S2);
    diffMprime(wk[2], wk[2]);

    for (i = 0; i < 16; ++i) tmp[i] = wk[2][i] ^ CK3[i];
    subBytes(tmp, wk[3], S1);
    diffM(wk[3], wk[3]);

    for (i = 0; i < 16; ++i) roundKey[0][i] = wk[0][i];
    rotBytes(wk[1], roundKey[1], 12);
    rotBytes(wk[2], roundKey[2], 8);
    rotBytes(wk[3], roundKey[3], 4);
    for (i = 0; i < 16; ++i) roundKey[4][i] = wk[0][i];
    rotBytes(wk[1], roundKey[5], 12);
    rotBytes(wk[2], roundKey[6], 8);
    rotBytes(wk[3], roundKey[7], 4);
    for (i = 0; i < 16; ++i) roundKey[8][i] = wk[0][i];
    rotBytes(wk[1], roundKey[9], 12);
    rotBytes(wk[2], roundKey[10], 8);
    rotBytes(wk[3], roundKey[11], 4);
    for (i = 0; i < 16; ++i) roundKey[12][i] = wk[0][i];
}

// ------------------------------------------------------------------
//  One round of encryption (type = 0 → FO, type = 1 → FE)
// ------------------------------------------------------------------
void encryptRound(int state[16], const int rk[16], int type) {
    int tmp[16];
    addRoundKey(state, rk);
    if (type == 0) {
        subBytes(state, tmp, S1);
        diffM(tmp, state);
    } else {
        subBytes(state, tmp, S2);
        diffMprime(tmp, state);
    }
}

// ------------------------------------------------------------------
//  Full ARIA‑128 encryption (12 rounds + final add‑key)
// ------------------------------------------------------------------
void aria128Encrypt(const int pt[16], const int masterKey[16], int ct[16]) {
    int roundKey[13][16];
    int state[16];
    int i;

    keySchedule(masterKey, roundKey);

    for (i = 0; i < 16; ++i) state[i] = pt[i] & 0xFF;

    for (i = 0; i < 12; ++i) {
        encryptRound(state, roundKey[i], i % 2);
    }

    addRoundKey(state, roundKey[12]);

    for (i = 0; i < 16; ++i) ct[i] = state[i] & 0xFF;
}

// ------------------------------------------------------------------
//  Helper: print 16‑byte block as hex
// ------------------------------------------------------------------
void printHex(const int block[16]) {
    int i;
    for (i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (block[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;
}

// ------------------------------------------------------------------
//  main – deterministic test vector (minimal‑boundary)
// ------------------------------------------------------------------
int main() {
    initSboxes();

    int key[16] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
    };
    int plain[16] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    int cipher[16];

    aria128Encrypt(plain, key, cipher);

    std::cout << "Plaintext : ";  printHex(plain);
    std::cout << "Key       : ";  printHex(key);
    std::cout << "Ciphertext: ";  printHex(cipher);
    return 0;
}
