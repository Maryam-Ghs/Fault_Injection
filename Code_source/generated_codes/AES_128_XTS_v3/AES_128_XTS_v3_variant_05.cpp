#include <iostream>
#include <iomanip>
#include <cstring>

// LLM input variant 5: duplicate-heavy
//------------------------------------------------------------
//  S‑box (filled with the standard AES values)
//------------------------------------------------------------
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

//------------------------------------------------------------
//  Helper arithmetic (GF(2^8) multiplication)
//------------------------------------------------------------
int mul2(int x) {
    int y = (x << 1) & 0xFF;
    if (x & 0x80) y ^= 0x1B;
    return y;
}
int mul3(int x) { return mul2(x) ^ x; }

//------------------------------------------------------------
//  AES core transformations (function based, reordered)
//------------------------------------------------------------
void sub_bytes(int *st) {
    int i = 0;
    while (i < 16) {
        st[i] = sbox[st[i] & 0xFF];
        i = i + 1;
    }
}
void shift_rows(int *st) {
    int tmp[16];
    int i = 0;
    while (i < 4) {
        tmp[i*4+0] = st[i*4+0];
        tmp[i*4+1] = st[(i*4+1+ i)%4 + 0];
        tmp[i*4+2] = st[(i*4+2+2*i)%4 + 0];
        tmp[i*4+3] = st[(i*4+3+3*i)%4 + 0];
        i = i + 1;
    }
    i = 0;
    while (i < 16) {
        st[i] = tmp[i];
        i = i + 1;
    }
}
void mix_columns(int *st) {
    int c = 0;
    while (c < 4) {
        int a0 = st[c];
        int a1 = st[4 + c];
        int a2 = st[8 + c];
        int a3 = st[12 + c];

        int r0 = mul2(a0) ^ a1 ^ a2 ^ mul3(a3);
        int r1 = mul3(a0) ^ mul2(a1) ^ a2 ^ a3;
        int r2 = a0 ^ mul3(a1) ^ mul2(a2) ^ a3;
        int r3 = a0 ^ a1 ^ mul3(a2) ^ mul2(a3);

        st[c]       = r0 & 0xFF;
        st[4 + c]   = r1 & 0xFF;
        st[8 + c]   = r2 & 0xFF;
        st[12 + c]  = r3 & 0xFF;
        c = c + 1;
    }
}
void add_round_key(int *st, int *rk) {
    int i = 0;
    while (i < 16) {
        st[i] ^= rk[i];
        i = i + 1;
    }
}

//------------------------------------------------------------
//  Key expansion for AES‑128 (11 round keys, 176 bytes)
//------------------------------------------------------------
void key_expand(const int *key, int *rk) {
    // copy original key as round 0
    int i = 0;
    while (i < 16) {
        rk[i] = key[i];
        i = i + 1;
    }

    int rcon = 1;
    i = 16;
    while (i < 176) {
        int temp[4];
        // copy previous word
        temp[0] = rk[i - 4];
        temp[1] = rk[i - 3];
        temp[2] = rk[i - 2];
        temp[3] = rk[i - 1];

        // every 16‑byte boundary apply core
        if ((i / 4) % 4 == 0) {
            // RotWord
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // SubWord
            temp[0] = sbox[temp[0] & 0xFF];
            temp[1] = sbox[temp[1] & 0xFF];
            temp[2] = sbox[temp[2] & 0xFF];
            temp[3] = sbox[temp[3] & 0xFF];
            // Rcon
            temp[0] ^= rcon;
            rcon = mul2(rcon);
        }

        // XOR with word 4 positions earlier
        rk[i]     = rk[i - 16] ^ temp[0];
        rk[i + 1] = rk[i - 15] ^ temp[1];
        rk[i + 2] = rk[i - 14] ^ temp[2];
        rk[i + 3] = rk[i - 13] ^ temp[3];
        i = i + 4;
    }
}

//------------------------------------------------------------
//  AES‑128 encrypt a single 16‑byte block (ECB)
//------------------------------------------------------------
void aes_encrypt_block(const int *inp, int *out, const int *rk) {
    int state[16];
    int i = 0;
    while (i < 16) {
        state[i] = inp[i];
        i = i + 1;
    }

    add_round_key(state, (int*)rk); // round 0

    int round = 1;
    while (round < 10) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, (int*)(rk + round * 16));
        round = round + 1;
    }

    // final round (no MixColumns)
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, (int*)(rk + 160));

    i = 0;
    while (i < 16) {
        out[i] = state[i] & 0xFF;
        i = i + 1;
    }
}

//------------------------------------------------------------
//  Multiply a 128‑bit tweak by α (x) in GF(2^128)
//------------------------------------------------------------
void tweak_mul_alpha(int *t) {
    int carry = 0;
    int i = 15;
    while (i >= 0) {
        int val = t[i];
        int new_carry = (val & 0x80) ? 1 : 0;
        t[i] = ((val << 1) & 0xFF) ^ carry;
        carry = new_carry;
        i = i - 1;
    }
    if (carry) t[15] ^= 0x87; // reduction polynomial
}

//------------------------------------------------------------
//  XTS encrypt (deterministic test vector, heap allocated)
//------------------------------------------------------------
void xts_encrypt(const int *pt, int *ct, int blk_cnt,
                 const int *k1, const int *k2, const int *tweak_plain) {
    // expand both keys
    int *rk1 = new int[176];
    int *rk2 = new int[176];
    key_expand(k1, rk1);
    key_expand(k2, rk2);

    // compute initial tweak = AES‑encrypt(k2, tweak_plain)
    int tweak[16];
    aes_encrypt_block(tweak_plain, tweak, rk2);

    int block = 0;
    while (block < blk_cnt) {
        // X = P_i XOR tweak
        int x[16];
        int i = 0;
        while (i < 16) {
            x[i] = pt[block * 16 + i] ^ tweak[i];
            i = i + 1;
        }
        // Y = AES‑encrypt(k1, X)
        int y[16];
        aes_encrypt_block(x, y, rk1);
        // C_i = Y XOR tweak
        i = 0;
        while (i < 16) {
            ct[block * 16 + i] = y[i] ^ tweak[i];
            i = i + 1;
        }
        // update tweak for next block (except after last)
        if (block != blk_cnt - 1) {
            tweak_mul_alpha(tweak);
        }
        block = block + 1;
    }

    delete[] rk1;
    delete[] rk2;
}

//------------------------------------------------------------
//  Main – deterministic test vectors, prints ciphertext
//------------------------------------------------------------
int main() {
    // deterministic keys, plaintext and sector number (tweak)
    int *key1 = new int[16];
    int *key2 = new int[16];
    int *plain = new int[32];
    int *cipher = new int[32];
    int *sector = new int[16];

    // key1 = all 0x01 (duplicate values)
    int i = 0;
    while (i < 16) {
        key1[i] = 0x01;
        i = i + 1;
    }
    // key2 = all 0x02 (duplicate values)
    i = 0;
    while (i < 16) {
        key2[i] = 0x02;
        i = i + 1;
    }
    // plaintext = all 0x03 (duplicate values, two identical blocks)
    i = 0;
    while (i < 32) {
        plain[i] = 0x03;
        i = i + 1;
    }
    // sector number = all zeros (duplicate-heavy)
    i = 0;
    while (i < 16) {
        sector[i] = 0;
        i = i + 1;
    }

    // encrypt 2 blocks (32 bytes)
    xts_encrypt(plain, cipher, 2, key1, key2, sector);

    // print result
    std::cout << "Ciphertext (hex):";
    i = 0;
    while (i < 32) {
        if (i % 16 == 0) std::cout << "\n";
        std::cout << std::hex << std::setfill('0') << std::setw(2)
                  << (cipher[i] & 0xFF) << " ";
        i = i + 1;
    }
    std::cout << std::dec << std::endl;

    delete[] key1;
    delete[] key2;
    delete[] plain;
    delete[] cipher;
    delete[] sector;
    return 0;
}
