/* LLM input variant 2: small-diverse */
// AES‑128‑CMAC implementation – version #10
// ------------------------------------------------------------
// All data are stored as signed int (0‑255 values).  No unsigned,
// double or long types are used.  Memory is allocated on the heap
// with new/delete and the program contains a self‑contained main().
// ------------------------------------------------------------

#include <iostream>
#include <iomanip>

// -----------------------------------------------------------------
// Helper: multiply by 2 in GF(2^8) (branch‑minimized using mask)
// -----------------------------------------------------------------
int mul2(int x) {
    int m = (x >> 7) & 1;                 // msb before shift
    int y = ((x << 1) & 0xFF);            // shift left & keep byte
    y ^= (0x1B * m);                      // conditional reduction
    return y & 0xFF;
}

// -----------------------------------------------------------------
// Helper: multiply by 3 = mul2(x) ^ x
// -----------------------------------------------------------------
int mul3(int x) {
    return (mul2(x) ^ x) & 0xFF;
}

// -----------------------------------------------------------------
// AES S‑box (static table, values fit in signed int)
// -----------------------------------------------------------------
int *sbox_table() {
    int *s = new int[256];
    int vals[256] = {
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
    for (int i = 0; i < 256; ++i) s[i] = vals[i];
    return s;
}

// -----------------------------------------------------------------
// Helper: XOR two 16‑byte blocks (in‑place)
// -----------------------------------------------------------------
void xor_block(int *dst, const int *src) {
    int i = 0;
    while (i < 16) {
        dst[i] ^= src[i];
        ++i;
    }
}

// -----------------------------------------------------------------
// AES round functions (all operate on 16‑byte state)
// -----------------------------------------------------------------
void sub_bytes(int *state, const int *sbox) {
    int i = 0;
    while (i < 16) {
        state[i] = sbox[state[i]];
        ++i;
    }
}
void shift_rows(int *st) {
    // row 1 (index 1,5,9,13) shift left by 1
    int t = st[1];
    st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = t;
    // row 2 shift left by 2
    t = st[2]; int u = st[6];
    st[2] = st[10]; st[6] = st[14]; st[10] = t; st[14] = u;
    // row 3 shift left by 3 (right by 1)
    t = st[15];
    st[15] = st[11]; st[11] = st[7]; st[7] = st[3]; st[3] = t;
}
void mix_columns(int *st) {
    int i = 0;
    while (i < 16) {
        int a0 = st[i];
        int a1 = st[i+1];
        int a2 = st[i+2];
        int a3 = st[i+3];
        int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
        int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
        int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
        int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
        st[i]   = r0 & 0xFF;
        st[i+1] = r1 & 0xFF;
        st[i+2] = r2 & 0xFF;
        st[i+3] = r3 & 0xFF;
        i += 4;
    }
}
void add_round_key(int *state, const int *roundKey, int round) {
    int i = 0;
    while (i < 16) {
        state[i] ^= roundKey[round*16 + i];
        ++i;
    }
}

// -----------------------------------------------------------------
// Key expansion – produces 44 words (4‑byte each) = 176 bytes
// -----------------------------------------------------------------
int *key_expansion(const int *key) {
    int *rk = new int[176];
    int i = 0;
    while (i < 16) {
        rk[i] = key[i];
        ++i;
    }
    int rcon = 1;
    i = 16;
    while (i < 176) {
        int temp[4];
        int j = 0;
        while (j < 4) {
            temp[j] = rk[i - 4 + j];
            ++j;
        }
        if ((i & 15) == 0) {
            // RotWord
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // SubWord
            int *sb = sbox_table();
            j = 0;
            while (j < 4) {
                temp[j] = sb[temp[j]];
                ++j;
            }
            delete[] sb;
            // Rcon
            temp[0] ^= rcon;
            rcon = mul2(rcon);
        }
        j = 0;
        while (j < 4) {
            rk[i] = rk[i - 16] ^ temp[j];
            ++i; ++j;
        }
    }
    return rk;
}

// -----------------------------------------------------------------
// AES‑128 encrypt a single 16‑byte block
// -----------------------------------------------------------------
void aes_encrypt_block(int *blk, const int *roundKey) {
    int *sb = sbox_table();
    sub_bytes(blk, sb);
    delete[] sb;
    add_round_key(blk, roundKey, 0);
    int round = 1;
    while (round < 10) {
        sub_bytes(blk, sbox_table());
        shift_rows(blk);
        mix_columns(blk);
        add_round_key(blk, roundKey, round);
        ++round;
    }
    sub_bytes(blk, sbox_table());
    shift_rows(blk);
    add_round_key(blk, roundKey, 10);
}

// -----------------------------------------------------------------
// Left shift a 128‑bit block by one bit (in‑place)
// -----------------------------------------------------------------
void lshift_one(int *blk) {
    int carry = 0, next;
    int i = 15;
    while (i >= 0) {
        next = (blk[i] >> 7) & 1;
        blk[i] = ((blk[i] << 1) & 0xFF) | carry;
        carry = next;
        --i;
    }
}

// -----------------------------------------------------------------
// Generate CMAC subkeys K1 and K2
// -----------------------------------------------------------------
void generate_subkeys(const int *roundKey, int *K1, int *K2) {
    int *zero = new int[16];
    int i = 0;
    while (i < 16) { zero[i] = 0; ++i; }
    aes_encrypt_block(zero, roundKey);          // L = AES_K(0^128)

    // K1 = L << 1
    for (i = 0; i < 16; ++i) K1[i] = zero[i];
    lshift_one(K1);
    if ((zero[0] >> 7) & 1) {
        K1[15] ^= 0x87;
    }

    // K2 = K1 << 1
    for (i = 0; i < 16; ++i) K2[i] = K1[i];
    lshift_one(K2);
    if ((K1[0] >> 7) & 1) {
        K2[15] ^= 0x87;
    }
    delete[] zero;
}

// -----------------------------------------------------------------
// CMAC calculation for an arbitrary message length (bytes)
// -----------------------------------------------------------------
void cmac_compute(const int *msg, int msgLen, const int *roundKey,
                  int *tagOut) {
    int *K1 = new int[16];
    int *K2 = new int[16];
    generate_subkeys(roundKey, K1, K2);

    int nBlocks = (msgLen + 15) / 16;               // ceil
    int lastComplete = (msgLen % 16) == 0 ? 1 : 0;
    int *Mlast = new int[16];
    int i = 0;
    while (i < 16) { Mlast[i] = 0; ++i; }

    if (lastComplete) {
        // last block is complete -> XOR with K1
        int offset = (nBlocks - 1) * 16;
        i = 0;
        while (i < 16) {
            Mlast[i] = msg[offset + i] ^ K1[i];
            ++i;
        }
    } else {
        // padding then XOR with K2
        int offset = (nBlocks - 1) * 16;
        i = 0;
        while (i < msgLen - offset) {
            Mlast[i] = msg[offset + i];
            ++i;
        }
        Mlast[i] = 0x80;                // padding 0x80...
        while (i < 16) {
            Mlast[i] ^= K2[i];
            ++i;
        }
    }

    int *X = new int[16];
    i = 0;
    while (i < 16) { X[i] = 0; ++i; }

    int blockIdx = 0;
    while (blockIdx < nBlocks - 1) {
        int *Y = new int[16];
        i = 0;
        while (i < 16) {
            Y[i] = X[i] ^ msg[blockIdx*16 + i];
            ++i;
        }
        aes_encrypt_block(Y, roundKey);
        i = 0;
        while (i < 16) { X[i] = Y[i]; ++i; }
        delete[] Y;
        ++blockIdx;
    }

    // final block
    i = 0;
    while (i < 16) {
        X[i] ^= Mlast[i];
        ++i;
    }
    aes_encrypt_block(X, roundKey);
    i = 0;
    while (i < 16) {
        tagOut[i] = X[i];
        ++i;
    }

    delete[] K1; delete[] K2; delete[] X; delete[] Mlast;
}

// -----------------------------------------------------------------
// Demo in main(): encrypt a static 12‑byte message and print CMAC
// -----------------------------------------------------------------
int main() {
    // 128‑bit key (example, varied)
    int *key = new int[16]{
        0x00,0x11,0x22,0x33, 0x44,0x55,0x66,0x77,
        0x88,0x99,0xaa,0xbb, 0xcc,0xdd,0xee,0xff
    };

    // Message (12 bytes, triggers padding path)
    int *msg = new int[12]{
        0x10,0x20,0x30,0x40,
        0x50,0x60,0x70,0x80,
        0x90,0xa0,0xb0,0xc0
    };
    int msgLen = 12;

    // Key schedule
    int *roundKey = key_expansion(key);

    // Compute CMAC
    int *tag = new int[16];
    cmac_compute(msg, msgLen, roundKey, tag);

    // Print tag as hex
    std::cout << "CMAC‑128: ";
    int i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (tag[i] & 0xFF);
        ++i;
    }
    std::cout << std::dec << std::endl;

    // Clean up
    delete[] key;
    delete[] msg;
    delete[] roundKey;
    delete[] tag;
    return 0;
}
