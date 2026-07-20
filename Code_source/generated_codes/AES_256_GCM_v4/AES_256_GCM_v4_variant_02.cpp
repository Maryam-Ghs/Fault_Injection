#include <iostream>
#include <iomanip>
#include <vector>

/* LLM input variant 2: small-diverse */

int main() {
    /*--------------------  Pre‑defined data  --------------------*/
    std::vector<int> plain = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };

    std::vector<int> key256 = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    };

    std::vector<int> aad = {
        0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x01,0x02
    };

    /*--------------------  S‑Box  --------------------*/
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

    /*--------------------  Rcon  --------------------*/
    std::vector<int> rcon = {
        0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
        0x6C,0xD8,0xAB,0x4D,0x9A
    };

    /*--------------------  Key expansion (AES‑256)  --------------------*/
    std::vector<int> schedule(240, 0);                 // 60 words * 4 bytes
    int i = 0;
    while (i < 32) {                                   // copy original key
        schedule[i] = key256[i];
        i = i + 1;
    }

    int bytesDone = 32;
    int rcIdx = 0;

    while (bytesDone < 240) {
        /* -----  take previous 4‑byte word  ----- */
        int t0 = schedule[bytesDone - 4];
        int t1 = schedule[bytesDone - 3];
        int t2 = schedule[bytesDone - 2];
        int t3 = schedule[bytesDone - 1];

        /* -----  RotWord  ----- */
        int tmp = t0; t0 = t1; t1 = t2; t2 = t3; t3 = tmp;

        /* -----  SubWord  ----- */
        t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];

        /* -----  Rcon (only on the first word of each 8‑word block)  ----- */
        t0 = t0 ^ rcon[rcIdx];
        rcIdx = rcIdx + 1;

        /* -----  First 4 bytes of new block  ----- */
        schedule[bytesDone + 0] = schedule[bytesDone - 32] ^ t0;
        schedule[bytesDone + 1] = schedule[bytesDone - 31] ^ t1;
        schedule[bytesDone + 2] = schedule[bytesDone - 30] ^ t2;
        schedule[bytesDone + 3] = schedule[bytesDone - 29] ^ t3;
        bytesDone = bytesDone + 4;

        /* -----  Remaining 3 words of the block  ----- */
        int j;
        j = 0;
        while (j < 3) {
            int s0 = schedule[bytesDone - 4];
            int s1 = schedule[bytesDone - 3];
            int s2 = schedule[bytesDone - 2];
            int s3 = schedule[bytesDone - 1];

            schedule[bytesDone + 0] = schedule[bytesDone - 32] ^ s0;
            schedule[bytesDone + 1] = schedule[bytesDone - 31] ^ s1;
            schedule[bytesDone + 2] = schedule[bytesDone - 30] ^ s2;
            schedule[bytesDone + 3] = schedule[bytesDone - 29] ^ s3;

            bytesDone = bytesDone + 4;
            j = j + 1;
        }

        /* -----  Next 4‑byte word needs SubWord again (because of 256‑bit key)  ----- */
        int w0 = schedule[bytesDone - 4];
        int w1 = schedule[bytesDone - 3];
        int w2 = schedule[bytesDone - 2];
        int w3 = schedule[bytesDone - 1];

        w0 = sbox[w0]; w1 = sbox[w1]; w2 = sbox[w2]; w3 = sbox[w3];

        schedule[bytesDone + 0] = schedule[bytesDone - 32] ^ w0;
        schedule[bytesDone + 1] = schedule[bytesDone - 31] ^ w1;
        schedule[bytesDone + 2] = schedule[bytesDone - 30] ^ w2;
        schedule[bytesDone + 3] = schedule[bytesDone - 29] ^ w3;

        bytesDone = bytesDone + 4;

        /* -----  Final three words of this 8‑word segment  ----- */
        j = 0;
        while (j < 3) {
            int u0 = schedule[bytesDone - 4];
            int u1 = schedule[bytesDone - 3];
            int u2 = schedule[bytesDone - 2];
            int u3 = schedule[bytesDone - 1];

            schedule[bytesDone + 0] = schedule[bytesDone - 32] ^ u0;
            schedule[bytesDone + 1] = schedule[bytesDone - 31] ^ u1;
            schedule[bytesDone + 2] = schedule[bytesDone - 30] ^ u2;
            schedule[bytesDone + 3] = schedule[bytesDone - 29] ^ u3;

            bytesDone = bytesDone + 4;
            j = j + 1;
        }
    }

    /*--------------------  Encryption of a single block  --------------------*/
    std::vector<int> state = plain;                     // copy plaintext

    /* -----  Initial AddRoundKey  ----- */
    int rk = 0;
    while (rk < 16) {
        state[rk] = state[rk] ^ schedule[rk];
        rk = rk + 1;
    }

    /* -----  13 full rounds (SubBytes, ShiftRows, MixColumns, AddRoundKey)  ----- */
    int round = 1;
    while (round <= 13) {
        /* SubBytes */
        int b0 = sbox[state[0]];  int b1 = sbox[state[1]];
        int b2 = sbox[state[2]];  int b3 = sbox[state[3]];
        int b4 = sbox[state[4]];  int b5 = sbox[state[5]];
        int b6 = sbox[state[6]];  int b7 = sbox[state[7]];
        int b8 = sbox[state[8]];  int b9 = sbox[state[9]];
        int b10 = sbox[state[10]];int b11 = sbox[state[11]];
        int b12 = sbox[state[12]];int b13 = sbox[state[13]];
        int b14 = sbox[state[14]];int b15 = sbox[state[15]];

        /* ShiftRows (manual unroll) */
        int s0 = b0;  int s1 = b5;  int s2 = b10; int s3 = b15;
        int s4 = b4;  int s5 = b9;  int s6 = b14; int s7 = b3;
        int s8 = b8;  int s9 = b13; int s10 = b2; int s11 = b7;
        int s12 = b12;int s13 = b1; int s14 = b6; int s15 = b11;

        /* MixColumns (manual, using xtime macro style) */
        auto xtime = [](int x) { return ((x << 1) ^ ((x & 0x80) ? 0x1b : 0)) & 0xFF; };

        int m0 = xtime(s0) ^ xtime(s1) ^ s1 ^ s2 ^ s3;
        int m1 = s0 ^ xtime(s1) ^ xtime(s2) ^ s2 ^ s3;
        int m2 = s0 ^ s1 ^ xtime(s2) ^ xtime(s3) ^ s3;
        int m3 = xtime(s0) ^ s0 ^ s1 ^ s2 ^ xtime(s3);

        int m4 = xtime(s4) ^ xtime(s5) ^ s5 ^ s6 ^ s7;
        int m5 = s4 ^ xtime(s5) ^ xtime(s6) ^ s6 ^ s7;
        int m6 = s4 ^ s5 ^ xtime(s6) ^ xtime(s7) ^ s7;
        int m7 = xtime(s4) ^ s4 ^ s5 ^ s6 ^ xtime(s7);

        int m8 = xtime(s8) ^ xtime(s9) ^ s9 ^ s10 ^ s11;
        int m9 = s8 ^ xtime(s9) ^ xtime(s10) ^ s10 ^ s11;
        int m10 = s8 ^ s9 ^ xtime(s10) ^ xtime(s11) ^ s11;
        int m11 = xtime(s8) ^ s8 ^ s9 ^ s10 ^ xtime(s11);

        int m12 = xtime(s12) ^ xtime(s13) ^ s13 ^ s14 ^ s15;
        int m13 = s12 ^ xtime(s13) ^ xtime(s14) ^ s14 ^ s15;
        int m14 = s12 ^ s13 ^ xtime(s14) ^ xtime(s15) ^ s15;
        int m15 = xtime(s12) ^ s12 ^ s13 ^ s14 ^ xtime(s15);

        /* AddRoundKey */
        int offset = round * 16;
        state[0]  = m0  ^ schedule[offset + 0];
        state[1]  = m1  ^ schedule[offset + 1];
        state[2]  = m2  ^ schedule[offset + 2];
        state[3]  = m3  ^ schedule[offset + 3];
        state[4]  = m4  ^ schedule[offset + 4];
        state[5]  = m5  ^ schedule[offset + 5];
        state[6]  = m6  ^ schedule[offset + 6];
        state[7]  = m7  ^ schedule[offset + 7];
        state[8]  = m8  ^ schedule[offset + 8];
        state[9]  = m9  ^ schedule[offset + 9];
        state[10] = m10 ^ schedule[offset +10];
        state[11] = m11 ^ schedule[offset +11];
        state[12] = m12 ^ schedule[offset +12];
        state[13] = m13 ^ schedule[offset +13];
        state[14] = m14 ^ schedule[offset +14];
        state[15] = m15 ^ schedule[offset +15];

        round = round + 1;
    }

    /* -----  Final round (no MixColumns)  ----- */
    /* SubBytes */
    int f0 = sbox[state[0]];  int f1 = sbox[state[1]];
    int f2 = sbox[state[2]];  int f3 = sbox[state[3]];
    int f4 = sbox[state[4]];  int f5 = sbox[state[5]];
    int f6 = sbox[state[6]];  int f7 = sbox[state[7]];
    int f8 = sbox[state[8]];  int f9 = sbox[state[9]];
    int f10 = sbox[state[10]];int f11 = sbox[state[11]];
    int f12 = sbox[state[12]];int f13 = sbox[state[13]];
    int f14 = sbox[state[14]];int f15 = sbox[state[15]];

    /* ShiftRows */
    state[0]  = f0;  state[1]  = f5;  state[2]  = f10; state[3]  = f15;
    state[4]  = f4;  state[5]  = f9;  state[6]  = f14; state[7]  = f3;
    state[8]  = f8;  state[9]  = f13; state[10] = f2;  state[11] = f7;
    state[12] = f12; state[13] = f1;  state[14] = f6;  state[15] = f11;

    /* AddRoundKey (round 14) */
    int finalOff = 14 * 16;
    int k = 0;
    while (k < 16) {
        state[k] = state[k] ^ schedule[finalOff + k];
        k = k + 1;
    }

    /*--------------------  Simple GCM‑like tag (XOR‑based)  --------------------*/
    std::vector<int> tag(16, 0);
    int t = 0;
    while (t < 16) {
        int aadByte = aad[t % 8];
        tag[t] = state[t] ^ aadByte;
        t = t + 1;
    }

    /*--------------------  Output  --------------------*/
    std::cout << "Ciphertext : ";
    int p = 0;
    while (p < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (state[p] & 0xFF) << ' ';
        p = p + 1;
    }
    std::cout << std::dec << "\nTag        : ";
    p = 0;
    while (p < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (tag[p] & 0xFF) << ' ';
        p = p + 1;
    }
    std::cout << std::dec << std::endl;
    return 0;
}
