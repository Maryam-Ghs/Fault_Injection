#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 4: signed-extremes */
int main() {
    /* ---------- small predefined key and plaintext (mixed signed extremes) ---------- */
    std::vector<int> key = {
        -128, 0x00, 0x7f, 0xff,
        -1,   0x80, 0x01, 0x7e,
        0x55, 0xaa, -64,  64,
        0x09, 0xcf, -32,  32
    };

    std::vector<int> plain = {
        0x32, -45, 0x00, 0xa8,
        0x88, 0x5a, -127, 0x8d,
        0x31, 0x00, 0x98, -2,
        0xe0, 0x37, 0x07, 0x34
    };

    /* ---------- S‑box and Rcon (non‑const, int only) ---------- */
    std::vector<int> sbox(256);
    std::vector<int> rcon(11);
    {
        int init_sbox[256] = {
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
        int init_rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};
        for (int i = 0; i < 256; ++i) sbox[i] = init_sbox[i];
        for (int i = 0; i < 11; ++i) rcon[i] = init_rcon[i];
    }

    /* ---------- key expansion (16 * (10+1) bytes) ---------- */
    std::vector<int> roundKey(176);
    for (int i = 0; i < 16; ++i) roundKey[i] = key[i];

    for (int i = 16; i < 176; i += 4) {
        int t0 = roundKey[i - 4];
        int t1 = roundKey[i - 3];
        int t2 = roundKey[i - 2];
        int t3 = roundKey[i - 1];

        if ((i / 4) % 4 == 0) {
            /* RotWord */
            int tmp = t0; t0 = t1; t1 = t2; t2 = t3; t3 = tmp;
            /* SubWord */
            t0 = sbox[t0 & 0xff];
            t1 = sbox[t1 & 0xff];
            t2 = sbox[t2 & 0xff];
            t3 = sbox[t3 & 0xff];
            /* Rcon */
            t0 ^= rcon[i / 16];
        }

        roundKey[i + 0] = roundKey[i - 16] ^ t0;
        roundKey[i + 1] = roundKey[i - 15] ^ t1;
        roundKey[i + 2] = roundKey[i - 14] ^ t2;
        roundKey[i + 3] = roundKey[i - 13] ^ t3;
    }

    /* ---------- helper lambdas for Galois multiplication ---------- */
    auto mul2 = [&](int x) {
        int r = (x & 0xff) << 1;
        if (x & 0x80) r ^= 0x1b;
        return r & 0xff;
    };
    auto mul3 = [&](int x) {
        return mul2(x) ^ (x & 0xff);
    };

    /* ---------- encryption (ECB, one block) ---------- */
    std::vector<int> state = plain;

    /* initial AddRoundKey */
    for (int i = 0; i < 16; ++i) {
        state[i] ^= roundKey[i];
        state[i] &= 0xff;
    }

    /* 9 main rounds */
    for (int round = 1; round <= 9; ++round) {
        /* SubBytes */
        for (int i = 0; i < 16; ++i) state[i] = sbox[state[i] & 0xff];

        /* ShiftRows (manual) */
        int s1 = state[1], s5 = state[5], s9 = state[9], s13 = state[13];
        state[1] = s5; state[5] = s9; state[9] = s13; state[13] = s1;

        int s2 = state[2], s6 = state[6], s10 = state[10], s14 = state[14];
        state[2] = s10; state[6] = s14; state[10] = s2; state[14] = s6;

        int s3 = state[3], s7 = state[7], s11 = state[11], s15 = state[15];
        state[3] = s15; state[7] = s3; state[11] = s7; state[15] = s11;

        /* MixColumns (manual, 4 columns) */
        for (int col = 0; col < 4; ++col) {
            int idx0 = col * 4 + 0;
            int idx1 = col * 4 + 1;
            int idx2 = col * 4 + 2;
            int idx3 = col * 4 + 3;

            int a0 = state[idx0];
            int a1 = state[idx1];
            int a2 = state[idx2];
            int a3 = state[idx3];

            int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);

            state[idx0] = r0;
            state[idx1] = r1;
            state[idx2] = r2;
            state[idx3] = r3;
        }

        /* AddRoundKey */
        int offset = round * 16;
        for (int i = 0; i < 16; ++i) {
            state[i] ^= roundKey[offset + i];
            state[i] &= 0xff;
        }
    }

    /* final round (no MixColumns) */
    for (int i = 0; i < 16; ++i) state[i] = sbox[state[i] & 0xff];

    int s1 = state[1], s5 = state[5], s9 = state[9], s13 = state[13];
    state[1] = s5; state[5] = s9; state[9] = s13; state[13] = s1;

    int s2 = state[2], s6 = state[6], s10 = state[10], s14 = state[14];
    state[2] = s10; state[6] = s14; state[10] = s2; state[14] = s6;

    int s3 = state[3], s7 = state[7], s11 = state[11], s15 = state[15];
    state[3] = s15; state[7] = s3; state[11] = s7; state[15] = s11;

    int finalOffset = 10 * 16;
    for (int i = 0; i < 16; ++i) state[i] ^= roundKey[finalOffset + i];

    /* ---------- output ciphertext ---------- */
    std::cout << "Ciphertext (hex): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (state[i] & 0xff);
    }
    std::cout << std::dec << std::endl;
    return 0;
}
