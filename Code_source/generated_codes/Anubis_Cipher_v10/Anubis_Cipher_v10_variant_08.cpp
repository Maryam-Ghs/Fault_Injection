#include <iostream>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

int main() {
    // version #10
    // ------------------------------------------------------------
    //  simple Anubis‑like cipher (128‑bit block, 128‑bit key)
    //  all data lives on the heap, no const, only int and float
    // ------------------------------------------------------------

    // ----- parameters ------------------------------------------------
    int R = 10;                               // number of rounds
    int B = 16;                               // block size in bytes

    // ----- S‑box (Anubis) --------------------------------------------
    static int S[256] = {
        0xBA,0x75,0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,
        0x72,0xC0,0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,
        0x31,0x15,0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,
        0xB2,0x75,0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,
        0x2F,0x84,0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,
        0x58,0xCF,0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,
        0x9F,0xA8,0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,
        0xF3,0xD2,0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,
        0x19,0x73,0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,
        0x0B,0xDB,0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,
        0xE4,0x79,0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,
        0xAE,0x08,0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,
        0x8B,0x8A,0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,
        0x1D,0x9E,0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,
        0x28,0xDF,0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,
        0xBB,0x16
    };

    // ----- MDS matrix (circulant) ------------------------------------
    static int M[4][4] = {
        {0x01,0x01,0x04,0x01},
        {0x01,0x01,0x01,0x04},
        {0x04,0x01,0x01,0x01},
        {0x01,0x04,0x01,0x01}
    };

    // ----- round constants (simple) ----------------------------------
    static int RC[11] = {
        0x00000000,0x13198a2e,0x03707344,0xa4093822,0x299f31d0,
        0x082efa98,0xec4e6c89,0x452821e6,0x38d01377,0xbe5466cf,
        0x34e90c6c
    };

    // ----- heap allocation --------------------------------------------
    int *plain = new int[B];
    int *key   = new int[B];
    int *state = new int[B];
    int *rk    = new int[(R+1)*B];   // round keys

    // ----- fill with a sparse, clustered pattern -----------------------
    for (int i=0;i<B;i++) {
        // Plaintext: non‑zero only at positions 0, 7, 15
        if (i==0 || i==7 || i==15) {
            plain[i] = (i*11 + 5) & 0xFF;
        } else {
            plain[i] = 0;
        }
        // Key: non‑zero only at positions 3, 8, 12
        if (i==3 || i==8 || i==12) {
            key[i] = (i*17 + 9) & 0xFF;
        } else {
            key[i] = 0;
        }
    }

    // ----- simple key schedule (rotate + XOR with RC) -----------------
    for (int i=0;i<B;i++) rk[i] = key[i];
    int t = 0;
    while (t < R) {
        // rotate left by 1 byte
        int tmp = rk[t*B];
        for (int i=0;i<B-1;i++) rk[(t+1)*B + i] = rk[t*B + i + 1];
        rk[(t+1)*B + B-1] = tmp;
        // XOR with round constant (low byte only, to stay in int)
        rk[(t+1)*B] ^= (RC[t+1] & 0xFF);
        t = t + 1;
    }

    // ----- copy plaintext to state ------------------------------------
    for (int i=0;i<B;i++) state[i] = plain[i];

    // ----- helper: multiplication by 2 in GF(2^8) (branch‑free) -------
    auto mul2 = [](int x)->int{
        return ((x << 1) ^ (0x1B & -(x >> 7))) & 0xFF;
    };
    // multiplication by 4 = mul2(mul2(x))
    auto mul4 = [&](int x)->int{
        return mul2(mul2(x));
    };

    // ----- encryption rounds -------------------------------------------
    int r = 0;
    while (r < R) {
        // SubBytes (S‑box)
        for (int i=0;i<B;i++) state[i] = S[state[i]];

        // ShiftRows (row i shifted left by i)
        int tmp[16];
        for (int c=0;c<4;c++) {
            for (int r2=0;r2<4;r2++) {
                int src = ((r2*4) + ((c + r2) & 3));
                tmp[r2*4 + c] = state[src];
            }
        }
        for (int i=0;i<B;i++) state[i] = tmp[i];

        // MixColumns (using MDS matrix)
        for (int c=0;c<4;c++) {
            int s0 = state[c];
            int s1 = state[4 + c];
            int s2 = state[8 + c];
            int s3 = state[12+ c];
            int m0 = ( (s0) ^ (s1) ^ (mul4(s2)) ^ (s3) ) & 0xFF;
            int m1 = ( (s0) ^ (s1) ^ (s2) ^ (mul4(s3)) ) & 0xFF;
            int m2 = ( (mul4(s0)) ^ (s1) ^ (s2) ^ (s3) ) & 0xFF;
            int m3 = ( (s0) ^ (mul4(s1)) ^ (s2) ^ (s3) ) & 0xFF;
            state[    c] = m0;
            state[4 + c] = m1;
            state[8 + c] = m2;
            state[12+ c] = m3;
        }

        // AddRoundKey
        for (int i=0;i<B;i++) state[i] ^= rk[(r+1)*B + i];

        r = r + 1;
    }

    // ----- final round (no MixColumns) ---------------------------------
    for (int i=0;i<B;i++) state[i] = S[state[i]];
    // ShiftRows once more
    int fin[16];
    for (int c=0;c<4;c++) {
        for (int r2=0;r2<4;r2++) {
            int src = ((r2*4) + ((c + r2) & 3));
            fin[r2*4 + c] = state[src];
        }
    }
    for (int i=0;i<B;i++) state[i] = fin[i] ^ rk[R*B + i];

    // ----- output -------------------------------------------------------
    std::cout << "Ciphertext (hex): ";
    for (int i=0;i<B;i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (state[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;

    // ----- clean up -----------------------------------------------------
    delete[] plain;
    delete[] key;
    delete[] state;
    delete[] rk;

    return 0;
}
