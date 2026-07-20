/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <iomanip>
#include <cstring>

// ---------------------------------------------------------------------------
// Global tables (no const, only int)
// ---------------------------------------------------------------------------
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

int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

// ---------------------------------------------------------------------------
// Helper utilities (heap allocation, XOR, copy, doubling)
// ---------------------------------------------------------------------------
void xor_block(int* a, int* b, int* out) {
    int i = 0;
    while (i < 16) {
        out[i] = a[i] ^ b[i];
        i = i + 1;
    }
}

void copy_block(int* src, int* dst) {
    int i = 0;
    while (i < 16) {
        dst[i] = src[i];
        i = i + 1;
    }
}

// GF(2^128) doubling (left shift, conditional xor 0x87)
void double_block(int* in, int* out) {
    int carry = 0;
    int i = 15;
    while (i >= 0) {
        int cur = in[i];
        int new_carry = (cur >> 7) & 1;
        out[i] = ((cur << 1) & 0xFF) ^ carry;
        carry = new_carry;
        i = i - 1;
    }
    if (carry) {
        out[15] = out[15] ^ 0x87;
    }
}

// ---------------------------------------------------------------------------
// AES core functions (each step broken into temporaries)
// ---------------------------------------------------------------------------
void add_round_key(int* state, int* roundKey) {
    int i = 0;
    while (i < 16) {
        state[i] = state[i] ^ roundKey[i];
        i = i + 1;
    }
}

void sub_bytes(int* state) {
    int i = 0;
    while (i < 16) {
        int idx = state[i];
        state[i] = sbox[idx];
        i = i + 1;
    }
}

void shift_rows(int* st) {
    int t1 = st[1];
    st[1] = st[5];
    st[5] = st[9];
    st[9] = st[13];
    st[13] = t1;
    int t2 = st[2];
    int t6 = st[6];
    st[2] = st[10];
    st[6] = st[14];
    st[10] = t2;
    st[14] = t6;
    int t3 = st[3];
    st[3] = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7] = t3;
}

// Multiplication by 2 in GF(2^8)
int mul2(int x) {
    int shifted = (x << 1) & 0xFF;
    if ((x & 0x80) != 0) shifted ^= 0x1B;
    return shifted;
}

// Multiplication by 3 = mul2(x) ^ x
int mul3(int x) {
    return mul2(x) ^ x;
}

void mix_columns(int* st) {
    int c = 0;
    while (c < 4) {
        int i0 = c * 4 + 0;
        int i1 = c * 4 + 1;
        int i2 = c * 4 + 2;
        int i3 = c * 4 + 3;

        int a0 = st[i0];
        int a1 = st[i1];
        int a2 = st[i2];
        int a3 = st[i3];

        int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
        int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
        int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
        int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);

        st[i0] = r0;
        st[i1] = r1;
        st[i2] = r2;
        st[i3] = r3;

        c = c + 1;
    }
}

// ---------------------------------------------------------------------------
// Key expansion for AES-128 (10 rounds, 44 words)
// ---------------------------------------------------------------------------
void aes_key_expand(int* keyBytes, int* roundKeyBytes) {
    int i = 0;
    while (i < 16) {
        roundKeyBytes[i] = keyBytes[i];
        i = i + 1;
    }

    int wordsGenerated = 4;
    int temp[4];
    while (wordsGenerated < 44) {
        int j = 0;
        while (j < 4) {
            temp[j] = roundKeyBytes[(wordsGenerated - 1) * 4 + j];
            j = j + 1;
        }

        if ((wordsGenerated % 4) == 0) {
            int rot = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = rot;
            int k = 0;
            while (k < 4) {
                temp[k] = sbox[temp[k]];
                k = k + 1;
            }
            temp[0] = temp[0] ^ rcon[wordsGenerated / 4];
        }

        int k = 0;
        while (k < 4) {
            roundKeyBytes[wordsGenerated * 4 + k] =
                roundKeyBytes[(wordsGenerated - 4) * 4 + k] ^ temp[k];
            k = k + 1;
        }

        wordsGenerated = wordsGenerated + 1;
    }
}

// ---------------------------------------------------------------------------
// AES block encryption (one 16‑byte block)
// ---------------------------------------------------------------------------
void aes_encrypt_block(int* inBlock, int* outBlock, int* roundKeyBytes) {
    int* state = new int[16];
    copy_block(inBlock, state);

    add_round_key(state, roundKeyBytes);

    int round = 1;
    while (round <= 9) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, roundKeyBytes + round * 16);
        round = round + 1;
    }

    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, roundKeyBytes + 10 * 16);

    copy_block(state, outBlock);
    delete[] state;
}

// ---------------------------------------------------------------------------
// OCB mode (simplified but functional for demonstration)
// ---------------------------------------------------------------------------
void ocb_encrypt(
    int* keyBytes,
    int* nonceBytes,
    int* plainBytes,
    int plainLen,
    int* cipherBytes,
    int* tagBytes)
{
    int* roundKeys = new int[176];
    aes_key_expand(keyBytes, roundKeys);

    int* zeroBlock = new int[16];
    int i = 0;
    while (i < 16) { zeroBlock[i] = 0; i = i + 1; }
    int* L = new int[16];
    aes_encrypt_block(zeroBlock, L, roundKeys);
    delete[] zeroBlock;

    int* Lstar = new int[16];
    int* Ldollar = new int[16];
    double_block(L, Lstar);
    double_block(Lstar, Ldollar);

    int* offset = new int[16];
    aes_encrypt_block(nonceBytes, offset, roundKeys);

    int* checksum = new int[16];
    i = 0;
    while (i < 16) { checksum[i] = 0; i = i + 1; }

    int processed = 0;
    while (processed + 16 <= plainLen) {
        xor_block(offset, L, offset);
        int* tmp = new int[16];
        xor_block(plainBytes + processed, offset, tmp);
        int* encTmp = new int[16];
        aes_encrypt_block(tmp, encTmp, roundKeys);
        xor_block(encTmp, offset, cipherBytes + processed);
        xor_block(checksum, plainBytes + processed, checksum);
        delete[] tmp;
        delete[] encTmp;
        processed = processed + 16;
    }

    int remaining = plainLen - processed;
    if (remaining > 0) {
        xor_block(offset, Lstar, offset);
        int* pad = new int[16];
        aes_encrypt_block(offset, pad, roundKeys);
        int j = 0;
        while (j < remaining) {
            cipherBytes[processed + j] = plainBytes[processed + j] ^ pad[j];
            j = j + 1;
        }
        int* padded = new int[16];
        i = 0;
        while (i < remaining) {
            padded[i] = plainBytes[processed + i];
            i = i + 1;
        }
        while (i < 16) {
            padded[i] = 0;
            i = i + 1;
        }
        xor_block(checksum, padded, checksum);
        delete[] pad;
        delete[] padded;
    }

    xor_block(offset, Ldollar, offset);
    int* tagInput = new int[16];
    xor_block(offset, checksum, tagInput);
    aes_encrypt_block(tagInput, tagBytes, roundKeys);

    delete[] roundKeys;
    delete[] L;
    delete[] Lstar;
    delete[] Ldollar;
    delete[] offset;
    delete[] checksum;
    delete[] tagInput;
}

// ---------------------------------------------------------------------------
// Main (generates edge‑case inputs, runs OCB, prints results)
// ---------------------------------------------------------------------------
int main() {
    int* keyBytes = new int[16];
    int i = 0;
    while (i < 16) { keyBytes[i] = 0; i = i + 1; }

    int* nonceBytes = new int[16];
    i = 0;
    while (i < 16) { nonceBytes[i] = 0; i = i + 1; }

    int plainLen = 0;
    int* plainBytes = new int[plainLen];
    i = 0;
    while (i < plainLen) {
        plainBytes[i] = 1; // fill with ones if any bytes existed
        i = i + 1;
    }

    int* cipherBytes = new int[plainLen];
    int* tagBytes = new int[16];

    ocb_encrypt(keyBytes, nonceBytes, plainBytes, plainLen, cipherBytes, tagBytes);

    std::cout << "Ciphertext (" << plainLen << " bytes): ";
    i = 0;
    while (i < plainLen) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipherBytes[i] & 0xFF);
        i = i + 1;
    }
    std::cout << std::dec << std::endl;

    std::cout << "Tag (16 bytes): ";
    i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (tagBytes[i] & 0xFF);
        i = i + 1;
    }
    std::cout << std::dec << std::endl;

    delete[] keyBytes;
    delete[] nonceBytes;
    delete[] plainBytes;
    delete[] cipherBytes;
    delete[] tagBytes;

    return 0;
}
