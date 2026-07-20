#include <iostream>
#include <iomanip>

/* LLM input variant 6: ordered-structured */

// -----------------------------------------------------------------
//  S‑box (fixed table)
// -----------------------------------------------------------------
int sbox[256] = {
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

// -----------------------------------------------------------------
//  Round constants for key schedule (Rcon)
// -----------------------------------------------------------------
int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

// -----------------------------------------------------------------
//  Multiply by 2 in GF(2^8)
// -----------------------------------------------------------------
int mul2(int x) {
    int y = x << 1;
    if (y & 0x100) y ^= 0x11B;
    return y & 0xFF;
}

// -----------------------------------------------------------------
//  Multiply by 3 in GF(2^8)
// -----------------------------------------------------------------
int mul3(int x) {
    return mul2(x) ^ x;
}

// -----------------------------------------------------------------
//  Helper: XOR two 16‑byte blocks into dst
// -----------------------------------------------------------------
void xor_block(int dst[16], const int a[16], const int b[16]) {
    int i = 0;
    while (i < 16) {
        dst[i] = a[i] ^ b[i];
        ++i;
    }
}

// -----------------------------------------------------------------
//  Key expansion: generate 11 round keys (each 16 bytes)
// -----------------------------------------------------------------
void expand_key(const int key[16], int roundKey[11][16]) {
    // copy first round key
    int i = 0;
    while (i < 16) {
        roundKey[0][i] = key[i];
        ++i;
    }

    int r = 1;
    while (r <= 10) {
        // temp = last word of previous round key
        int temp[4];
        int j = 0;
        while (j < 4) {
            temp[j] = roundKey[r-1][12 + j];
            ++j;
        }
        // RotWord
        int t0 = temp[0];
        temp[0] = temp[1];
        temp[1] = temp[2];
        temp[2] = temp[3];
        temp[3] = t0;
        // SubWord
        j = 0;
        while (j < 4) {
            temp[j] = sbox[temp[j]];
            ++j;
        }
        // Rcon
        temp[0] ^= rcon[r];

        // generate 16 bytes of this round key
        i = 0;
        while (i < 16) {
            int src = roundKey[r-1][i] ^ temp[i % 4];
            roundKey[r][i] = src;
            // prepare temp for next word
            if ((i % 4) == 3) {
                int k = 0;
                while (k < 4) {
                    temp[k] = roundKey[r][i - 3 + k];
                    ++k;
                }
            }
            ++i;
        }
        ++r;
    }
}

// -----------------------------------------------------------------
//  SubBytes step
// -----------------------------------------------------------------
void sub_bytes(int state[16]) {
    int i = 0;
    while (i < 16) {
        state[i] = sbox[state[i]];
        ++i;
    }
}

// -----------------------------------------------------------------
//  ShiftRows step (state is column‑major)
// -----------------------------------------------------------------
void shift_rows(int state[16]) {
    int tmp[16];
    int r = 0;
    while (r < 4) {
        int c = 0;
        while (c < 4) {
            tmp[r + 4*c] = state[r + 4*((c + r) % 4)];
            ++c;
        }
        ++r;
    }
    r = 0;
    while (r < 16) {
        state[r] = tmp[r];
        ++r;
    }
}

// -----------------------------------------------------------------
//  MixColumns step
// -----------------------------------------------------------------
void mix_columns(int state[16]) {
    int c = 0;
    while (c < 4) {
        int a0 = state[0 + 4*c];
        int a1 = state[1 + 4*c];
        int a2 = state[2 + 4*c];
        int a3 = state[3 + 4*c];

        int b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
        int b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
        int b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
        int b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);

        state[0 + 4*c] = b0;
        state[1 + 4*c] = b1;
        state[2 + 4*c] = b2;
        state[3 + 4*c] = b3;
        ++c;
    }
}

// -----------------------------------------------------------------
//  AddRoundKey step
// -----------------------------------------------------------------
void add_round_key(int state[16], const int rkey[16]) {
    int i = 0;
    while (i < 16) {
        state[i] ^= rkey[i];
        ++i;
    }
}

// -----------------------------------------------------------------
//  AES‑128 block encryption (16 bytes)
// -----------------------------------------------------------------
void aes_encrypt(const int in[16], int out[16], const int roundKey[11][16]) {
    int state[16];
    int i = 0;
    while (i < 16) {
        state[i] = in[i];
        ++i;
    }

    add_round_key(state, roundKey[0]);

    int round = 1;
    while (round < 10) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, roundKey[round]);
        ++round;
    }

    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, roundKey[10]);

    i = 0;
    while (i < 16) {
        out[i] = state[i];
        ++i;
    }
}

// -----------------------------------------------------------------
//  Very small OCB‑like wrapper (single‑block, deterministic)
// -----------------------------------------------------------------
void ocb_one_block(const int key[16],
                   const int nonce[12],
                   const int plain[16],
                   int cipher[16],
                   int tag[16]) {
    // 1. expand key
    int roundKey[11][16];
    expand_key(key, roundKey);

    // 2. build nonce block = nonce || 0x00 0x00 0x00 0x01
    int nblk[16];
    int i = 0;
    while (i < 12) {
        nblk[i] = nonce[i];
        ++i;
    }
    nblk[12] = 0; nblk[13] = 0; nblk[14] = 0; nblk[15] = 1;

    // 3. offset = AES_K(nblk)
    int offset[16];
    aes_encrypt(nblk, offset, roundKey);

    // 4. ciphertext = plaintext XOR offset
    xor_block(cipher, plain, offset);

    // 5. tag = AES_K(cipher XOR offset)
    int tmp[16];
    xor_block(tmp, cipher, offset);
    aes_encrypt(tmp, tag, roundKey);
}

// -----------------------------------------------------------------
//  Print a 16‑byte block as hex
// -----------------------------------------------------------------
void print_hex(const char *label, const int blk[16]) {
    std::cout << label;
    int i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (blk[i] & 0xFF);
        ++i;
    }
    std::cout << std::dec << std::endl;
}

// -----------------------------------------------------------------
//  Main – deterministic test vector with ordered‑structured data
// -----------------------------------------------------------------
int main() {
    // key = 0,1,2,3,4,5,6,7,7,6,5,4,3,2,1,0 (symmetric)
    int key[16];
    int i = 0;
    while (i < 16) {
        if (i < 8) key[i] = i;
        else key[i] = 15 - i;
        ++i;
    }

    // nonce = 0,1,2,3,4,5,5,4,3,2,1,0 (palindromic)
    int nonce[12];
    i = 0;
    while (i < 12) {
        if (i < 6) nonce[i] = i;
        else nonce[i] = 11 - i;
        ++i;
    }

    // plaintext = 0x00 … 0x0F (strictly ordered)
    int plain[16];
    i = 0;
    while (i < 16) {
        plain[i] = i;
        ++i;
    }

    int cipher[16];
    int tag[16];

    ocb_one_block(key, nonce, plain, cipher, tag);

    print_hex("Ciphertext: ", cipher);
    print_hex("Tag       : ", tag);

    return 0;
}
