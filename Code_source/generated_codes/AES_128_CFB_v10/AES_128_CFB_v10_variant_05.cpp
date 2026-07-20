#include <iostream>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */

// -----------------------------------------------------------------
//  AES-128 implementation (encryption only) – CFB mode
//  Constraints: only int / float, no const, no unsigned, no double
// -----------------------------------------------------------------

// -----------------------------------------------------------------
//  S‑Box (standard AES)
// -----------------------------------------------------------------
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

// -----------------------------------------------------------------
//  Round constant (Rcon)
// -----------------------------------------------------------------
int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

// -----------------------------------------------------------------
//  Helper: multiply in GF(2^8) by 2 (used in MixColumns)
// -----------------------------------------------------------------
int xtime(int x) {
    int shifted = (x << 1) & 0xFF;
    return (x & 0x80) ? (shifted ^ 0x1b) : shifted;
}

// -----------------------------------------------------------------
//  SubBytes – manual unrolled
// -----------------------------------------------------------------
void SubBytes(int *st) {
    st[0] = sbox[st[0]]; st[1] = sbox[st[1]]; st[2] = sbox[st[2]]; st[3] = sbox[st[3]];
    st[4] = sbox[st[4]]; st[5] = sbox[st[5]]; st[6] = sbox[st[6]]; st[7] = sbox[st[7]];
    st[8] = sbox[st[8]]; st[9] = sbox[st[9]]; st[10] = sbox[st[10]]; st[11] = sbox[st[11]];
    st[12] = sbox[st[12]]; st[13] = sbox[st[13]]; st[14] = sbox[st[14]]; st[15] = sbox[st[15]];
}

// -----------------------------------------------------------------
//  ShiftRows – manual unrolled
// -----------------------------------------------------------------
void ShiftRows(int *st) {
    int t;

    // Row 1 – shift left by 1
    t = st[1]; st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = t;

    // Row 2 – shift left by 2
    t = st[2]; st[2] = st[10]; st[10] = t;
    t = st[6]; st[6] = st[14]; st[14] = t;

    // Row 3 – shift left by 3 (right by 1)
    t = st[15]; st[15] = st[11]; st[11] = st[7]; st[7] = st[3]; st[3] = t;
}

// -----------------------------------------------------------------
//  MixColumns – manual unrolled (full column operation)
// -----------------------------------------------------------------
void MixColumns(int *st) {
    int i, a0, a1, a2, a3, r0, r1, r2, r3;

    i = 0;
    while (i < 16) {
        a0 = st[i]; a1 = st[i+1]; a2 = st[i+2]; a3 = st[i+3];
        r0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        r1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        r2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        r3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        st[i]   = r0 & 0xFF;
        st[i+1] = r1 & 0xFF;
        st[i+2] = r2 & 0xFF;
        st[i+3] = r3 & 0xFF;
        i += 4;
    }
}

// -----------------------------------------------------------------
//  AddRoundKey – manual unrolled
// -----------------------------------------------------------------
void AddRoundKey(int *st, int *rk) {
    st[0] ^= rk[0];  st[1] ^= rk[1];  st[2] ^= rk[2];  st[3] ^= rk[3];
    st[4] ^= rk[4];  st[5] ^= rk[5];  st[6] ^= rk[6];  st[7] ^= rk[7];
    st[8] ^= rk[8];  st[9] ^= rk[9];  st[10] ^= rk[10]; st[11] ^= rk[11];
    st[12] ^= rk[12]; st[13] ^= rk[13]; st[14] ^= rk[14]; st[15] ^= rk[15];
}

// -----------------------------------------------------------------
//  Key expansion – produces 176‑byte round key schedule
// -----------------------------------------------------------------
void ExpandKey(int *key, int *schedule) {
    int i = 0;
    // first 16 bytes are the original key
    while (i < 16) {
        schedule[i] = key[i];
        ++i;
    }

    int bytesGenerated = 16;
    int rconIter = 1;
    int temp[4];

    while (bytesGenerated < 176) {
        // take last 4 bytes
        temp[0] = schedule[bytesGenerated - 4];
        temp[1] = schedule[bytesGenerated - 3];
        temp[2] = schedule[bytesGenerated - 2];
        temp[3] = schedule[bytesGenerated - 1];

        // every 16 bytes apply core
        if ((bytesGenerated % 16) == 0) {
            // rotate left
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // sbox
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
            // Rcon
            temp[0] ^= rcon[rconIter];
            ++rconIter;
        }

        // XOR with 4‑byte block 16 bytes earlier
        schedule[bytesGenerated]     = schedule[bytesGenerated - 16] ^ temp[0];
        schedule[bytesGenerated + 1] = schedule[bytesGenerated - 15] ^ temp[1];
        schedule[bytesGenerated + 2] = schedule[bytesGenerated - 14] ^ temp[2];
        schedule[bytesGenerated + 3] = schedule[bytesGenerated - 13] ^ temp[3];

        bytesGenerated += 4;
    }
}

// -----------------------------------------------------------------
//  AES block encryption (128‑bit)
// -----------------------------------------------------------------
void EncryptBlock(int *inBlock, int *outBlock, int *schedule) {
    int state[16];
    int round = 0;
    int *rkPtr = schedule;

    // copy input to state
    int i = 0;
    while (i < 16) {
        state[i] = inBlock[i];
        ++i;
    }

    // initial AddRoundKey
    AddRoundKey(state, rkPtr);
    rkPtr += 16;
    round = 1;

    // 9 main rounds
    while (round < 10) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, rkPtr);
        rkPtr += 16;
        ++round;
    }

    // final round (no MixColumns)
    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, rkPtr);

    // copy to output
    i = 0;
    while (i < 16) {
        outBlock[i] = state[i] & 0xFF;
        ++i;
    }
}

// -----------------------------------------------------------------
//  CFB‑128 encryption (full‑block feedback)
// -----------------------------------------------------------------
void CFB128Encrypt(int *plain, int *cipher, int len, int *key, int *iv) {
    int roundKeys[176];
    ExpandKey(key, roundKeys);

    int feedback[16];
    int i = 0;
    while (i < 16) {
        feedback[i] = iv[i];
        ++i;
    }

    int processed = 0;
    while (processed < len) {
        int encrypted[16];
        EncryptBlock(feedback, encrypted, roundKeys);

        // XOR plaintext with encrypted feedback
        i = 0;
        while (i < 16) {
            cipher[processed + i] = plain[processed + i] ^ encrypted[i];
            ++i;
        }

        // feedback becomes ciphertext block
        i = 0;
        while (i < 16) {
            feedback[i] = cipher[processed + i];
            ++i;
        }

        processed += 16;
    }
}

// -----------------------------------------------------------------
//  Main – small predefined data, prints ciphertext in hex
// -----------------------------------------------------------------
int main() {
    // 16‑byte key filled with a repeated pattern (0xAA)
    int key[16] = {
        0xAA,0xAA,0xAA,0xAA,
        0xAA,0xAA,0xAA,0xAA,
        0xAA,0xAA,0xAA,0xAA,
        0xAA,0xAA,0xAA,0xAA
    };

    // 16‑byte IV (all zeros) – already duplicate-heavy
    int iv[16] = {
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };

    // 16‑byte plaintext filled with the same byte (0x41, 'A')
    int plain[16] = {
        0x41,0x41,0x41,0x41,
        0x41,0x41,0x41,0x41,
        0x41,0x41,0x41,0x41,
        0x41,0x41,0x41,0x41
    };

    int cipher[16];
    CFB128Encrypt(plain, cipher, 16, key, iv);

    std::cout << "Ciphertext (hex): ";
    int i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xFF);
        ++i;
    }
    std::cout << std::dec << std::endl;
    return 0;
}
