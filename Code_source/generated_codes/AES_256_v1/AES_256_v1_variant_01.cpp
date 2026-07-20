/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <iomanip>

// ------------------------------------------------------------
// S‑box (filled with the standard AES values)
int sbox[256] = {
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

// Rcon values (only the first byte is non‑zero)
int rcon[15] = {
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8,0xAB,0x4D,0x9A
};

// -----------------------------------------------------------------
// Finite‑field multiplication helpers (only 2 and 3 are needed)
int xtime(int x) {
    int shifted = (x << 1) & 0xFF;
    int reduced = ((x >> 7) & 1) * 0x1B;
    return (shifted ^ reduced) & 0xFF;
}
int mul2(int x) { return xtime(x); }
int mul3(int x) { return xtime(x) ^ x; }

// -----------------------------------------------------------------
// SubBytes – fully unrolled
void subBytes(int *st) {
    st[0] = sbox[st[0]];  st[1] = sbox[st[1]];  st[2] = sbox[st[2]];  st[3] = sbox[st[3]];
    st[4] = sbox[st[4]];  st[5] = sbox[st[5]];  st[6] = sbox[st[6]];  st[7] = sbox[st[7]];
    st[8] = sbox[st[8]];  st[9] = sbox[st[9]];  st[10]= sbox[st[10]]; st[11]= sbox[st[11]];
    st[12]= sbox[st[12]]; st[13]= sbox[st[13]]; st[14]= sbox[st[14]]; st[15]= sbox[st[15]];
}

// -----------------------------------------------------------------
// ShiftRows – fully unrolled
void shiftRows(int *st) {
    int t;
    // row 1 (indices 1,5,9,13) left rotate 1
    t = st[1]; st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = t;
    // row 2 (2,6,10,14) left rotate 2
    t = st[2]; st[2] = st[10]; st[10] = t;
    t = st[6]; st[6] = st[14]; st[14] = t;
    // row 3 (3,7,11,15) left rotate 3 (right rotate 1)
    t = st[15]; st[15] = st[11]; st[11] = st[7]; st[7] = st[3]; st[3] = t;
}

// -----------------------------------------------------------------
// MixColumns – fully unrolled, using fused expressions
void mixColumns(int *st) {
    int a0,a1,a2,a3,r0,r1,r2,r3;

    // column 0
    a0=st[0]; a1=st[4]; a2=st[8]; a3=st[12];
    r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
    r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
    r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
    r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
    st[0]=r0; st[4]=r1; st[8]=r2; st[12]=r3;

    // column 1
    a0=st[1]; a1=st[5]; a2=st[9]; a3=st[13];
    r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
    r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
    r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
    r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
    st[1]=r0; st[5]=r1; st[9]=r2; st[13]=r3;

    // column 2
    a0=st[2]; a1=st[6]; a2=st[10]; a3=st[14];
    r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
    r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
    r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
    r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
    st[2]=r0; st[6]=r1; st[10]=r2; st[14]=r3;

    // column 3
    a0=st[3]; a1=st[7]; a2=st[11]; a3=st[15];
    r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
    r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
    r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
    r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
    st[3]=r0; st[7]=r1; st[11]=r2; st[15]=r3;
}

// -----------------------------------------------------------------
// AddRoundKey – fully unrolled, fused XOR
void addRoundKey(int *st, int *rk) {
    st[0] ^= rk[0];   st[1] ^= rk[1];   st[2] ^= rk[2];   st[3] ^= rk[3];
    st[4] ^= rk[4];   st[5] ^= rk[5];   st[6] ^= rk[6];   st[7] ^= rk[7];
    st[8] ^= rk[8];   st[9] ^= rk[9];   st[10]^= rk[10];  st[11]^= rk[11];
    st[12]^= rk[12];  st[13]^= rk[13];  st[14]^= rk[14];  st[15]^= rk[15];
}

// -----------------------------------------------------------------
// Key expansion for AES‑256 – operates on byte arrays
void keyExpansion(int *keyBytes, int *exp) {
    const int Nk = 8;   // 256‑bit key = 8 words
    const int Nb = 4;
    const int Nr = 14;
    int i, j;
    // first Nk words are just the key
    for (i = 0; i < Nk; ++i) {
        for (j = 0; j < 4; ++j)
            exp[4*i + j] = keyBytes[4*i + j];
    }
    int temp[4];
    for (i = Nk; i < Nb*(Nr+1); ++i) {
        // copy previous word into temp
        for (j = 0; j < 4; ++j)
            temp[j] = exp[4*(i-1) + j];
        if (i % Nk == 0) {
            // RotWord
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // SubWord
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
            // Rcon
            temp[0] ^= rcon[i/Nk];
        } else if (Nk > 6 && (i % Nk) == 4) {
            // SubWord only
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
        }
        // XOR with word Nk positions earlier
        for (j = 0; j < 4; ++j)
            exp[4*i + j] = exp[4*(i-Nk) + j] ^ temp[j];
    }
}

// -----------------------------------------------------------------
int main() {
    // 256‑bit key (32 bytes) – minimal boundary values (all zeros)
    int key[32] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    // One block of plaintext (16 bytes) – minimal boundary values (all zeros)
    int plain[16] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    // State and expanded key live on the stack
    int state[16];
    int roundKey[240];   // 60 words × 4 bytes

    // initialise state
    for (int i = 0; i < 16; ++i) state[i] = plain[i];

    // key schedule
    keyExpansion(key, roundKey);

    // ---- AES‑256 encryption ----
    addRoundKey(state, roundKey);                 // round 0

    for (int r = 1; r < 14; ++r) {                // rounds 1 … 13
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, roundKey + r*16);
    }

    // final round (no MixColumns)
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, roundKey + 14*16);

    // output ciphertext as hex string
    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i) {
        int b = state[i] & 0xFF;
        if (b < 16) std::cout << '0';
        std::cout << std::hex << b;
    }
    std::cout << std::endl;
    return 0;
}
