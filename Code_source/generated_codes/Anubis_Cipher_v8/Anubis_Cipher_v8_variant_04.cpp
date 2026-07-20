#include <iostream>
#include <iomanip>

/* LLM input variant 4: signed-extremes */

int main() {
    /*------------------------------------------------------------
     *  Anubis Cipher – single‑block encryption (128‑bit key)
     *  All data are stored in plain int arrays on the stack.
     *  No branches are used in the core transformations.
     *------------------------------------------------------------*/

    /*--- S‑box (values taken from the Anubis specification) ---*/
    int sbox[256] = {
        0xB6,0x2A,0xB5,0x3D,0xCC,0xBC,0x5E,0x1A,0xA2,0x25,0x4C,0x97,0xD3,0x6E,0x53,0x5F,
        0xC0,0xDB,0x0F,0x5C,0xC2,0xC8,0xF6,0x54,0xD5,0x8E,0x84,0x21,0x8B,0xF5,0x00,0xA7,
        0xD9,0x5B,0xD2,0x1D,0x9C,0x70,0x74,0xE9,0x36,0x7D,0xF0,0x2A,0x5A,0x3E,0xF4,0x1C,
        0x1B,0x5D,0xFD,0xC6,0xA5,0x4D,0xB8,0x3F,0x99,0x73,0xB9,0x5C,0x1E,0x6D,0xA8,0x2E,
        0xA6,0xF3,0x32,0xC4,0xEF,0xB2,0x5E,0x3C,0x33,0x4B,0x5A,0xA0,0xDE,0x8F,0x1F,0xD5,
        0x0D,0xC1,0xC9,0xB0,0x2D,0xE5,0xCB,0x4F,0xB9,0x7F,0x56,0x6A,0x48,0x5B,0x0E,0xD6,
        0x26,0x6F,0x74,0xC5,0x6B,0x0A,0x96,0x4A,0x45,0x73,0x8C,0x20,0x5F,0xB3,0xE4,0xF8,
        0xC7,0x41,0x2C,0x76,0x3A,0x5C,0xB7,0x52,0x07,0xE1,0x18,0x91,0xA1,0x5D,0xED,0xEB,
        0xF2,0x86,0xA3,0xC3,0xF1,0xC2,0x4D,0x0C,0x6C,0x15,0x33,0x8A,0xF9,0xD7,0xF7,0xBE,
        0x1A,0x6F,0xE6,0x7E,0x8D,0xA9,0x9F,0x3B,0xA4,0x2F,0x0B,0xD0,0x89,0xF0,0xB1,0x41,
        0x57,0xE7,0xAF,0xC9,0xE8,0x4C,0x6E,0x5A,0x9F,0x48,0x2C,0xE5,0x0F,0xD3,0x61,0x0B,
        0x7B,0xA0,0xB5,0xCC,0x7C,0x75,0xCA,0x0D,0x99,0x9A,0x2D,0x8F,0xA2,0x70,0x1C,0x3F,
        0x31,0x6B,0x86,0xC2,0x9D,0x31,0x0A,0x04,0x3E,0xC3,0x5F,0x89,0xA3,0xD5,0xC8,0xFA,
        0x6D,0xAA,0x9E,0xD9,0x62,0x9C,0x6A,0x0E,0x73,0x0C,0xC5,0xE0,0x01,0x7F,0x40,0xD8,
        0x22,0x52,0x57,0xF3,0x4E,0x5C,0x0A,0x7A,0x5B,0xD4,0x7D,0x90,0x0C,0x44,0xB0,0x27,
        0xB2,0xE4,0xCF,0xF9,0x2B,0xB6,0xF0,0x01,0x3D,0x9C,0x6E,0x44,0x9B,0x2A,0xC0,0x4F
    };

    /*--- Round constants (12 rounds) --------------------------------*/
    int rcon[12] = {
        0x01010101, 0x02020202, 0x04040404, 0x08080808,
        0x10010101, 0x20020202, 0x40040404, 0x80080808,
        0x1B010101, 0x36020202, 0x6C040404, 0xD8080808
    };

    /*--- GF(2^8) multiplication helpers (branch‑free) ---------------*/
    auto mul2 = [](int x)->int {
        int shifted = (x << 1) & 0xFF;
        int mask    = ((x >> 7) & 1) * 0x1B;
        return shifted ^ mask;
    };
    auto mul3 = [&](int x)->int { return mul2(x) ^ (x & 0xFF); };

    /*--- Test vectors (signed‑extreme mix) --------------------------*/
    // Mixed key: zeros, low positives, high positives, full 0xFF
    int keyMixed[16] = {
        0x00, 0x7F, 0x80, 0xFF,
        0x01, 0xFE, 0x55, 0xAA,
        0x10, 0xEF, 0x20, 0xD0,
        0x30, 0xC0, 0x40, 0xB0
    };
    // Plaintext all zero (baseline)
    int ptZero[16];   for(int i=0;i<16;i++) ptZero[i]=0;
    // Plaintext with negative, zero, and positive values
    int ptMixed[16] = {
        -1, -128, -64, -32,
        -16, -8, -4, -2,
        -1, 0, 1, 2,
        3, 4, 5, 6
    };
    // Full‑FF key (positive extreme)
    int keyFull[16];   for(int i=0;i<16;i++) keyFull[i]=0xFF;

    /*--- Helper: expand a 128‑bit key to (12+1) round keys ---------*/
    auto expandKey = [&](int *keyBytes, int w[4*13]) {
        /* w[0..3] = key (big‑endian word construction) */
        for(int i=0;i<4;i++){
            w[i] = (keyBytes[4*i] << 24) | (keyBytes[4*i+1] << 16) |
                   (keyBytes[4*i+2] << 8) | keyBytes[4*i+3];
        }
        for(int i=4;i<4*13;i++){
            int temp = w[i-1];
            if(i%4==0){
                /* RotWord */
                temp = ((temp << 8) | ((temp >> 24) & 0xFF)) & 0xFFFFFFFF;
                /* SubWord */
                int b0 = sbox[(temp>>24)&0xFF];
                int b1 = sbox[(temp>>16)&0xFF];
                int b2 = sbox[(temp>>8 )&0xFF];
                int b3 = sbox[(temp    )&0xFF];
                temp = (b0<<24)|(b1<<16)|(b2<<8)|b3;
                /* XOR round constant */
                temp ^= rcon[(i/4)-1];
            }
            w[i] = w[i-4] ^ temp;
        }
    };

    /*--- Core encryption routine (in‑place on 4×4 state) ------------*/
    auto encryptBlock = [&](int *inBytes, int *outBytes, int *roundKeyWords){
        int state[4][4];
        /* Load plaintext (column‑major) */
        for(int c=0;c<4;c++) for(int r=0;r<4;r++) state[r][c]=inBytes[4*c+r] & 0xFF;

        /* Initial AddRoundKey */
        for(int c=0;c<4;c++){
            int wk = roundKeyWords[c];
            for(int r=0;r<4;r++){
                int byte = (wk >> (24-8*r)) & 0xFF;
                state[r][c] ^= byte;
            }
        }

        /* 11 full rounds */
        for(int round=1; round<=11; round++){
            /* SubBytes */
            for(int r=0;r<4;r++) for(int c=0;c<4;c++) state[r][c]=sbox[state[r][c]];
            /* ShiftRows */
            for(int r=1;r<4;r++){
                int tmp[4];
                for(int c=0;c<4;c++) tmp[c]=state[r][(c+r)&3];
                for(int c=0;c<4;c++) state[r][c]=tmp[c];
            }
            /* MixColumns */
            for(int c=0;c<4;c++){
                int a0=state[0][c], a1=state[1][c], a2=state[2][c], a3=state[3][c];
                int b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
                int b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
                int b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
                int b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
                state[0][c]=b0&0xFF; state[1][c]=b1&0xFF;
                state[2][c]=b2&0xFF; state[3][c]=b3&0xFF;
            }
            /* AddRoundKey */
            for(int c=0;c<4;c++){
                int wk = roundKeyWords[4*round + c];
                for(int r=0;r<4;r++){
                    int byte = (wk >> (24-8*r)) & 0xFF;
                    state[r][c] ^= byte;
                }
            }
        }

        /* Final round (no MixColumns) */
        for(int r=0;r<4;r++) for(int c=0;c<4;c++) state[r][c]=sbox[state[r][c]];
        for(int r=1;r<4;r++){
            int tmp[4];
            for(int c=0;c<4;c++) tmp[c]=state[r][(c+r)&3];
            for(int c=0;c<4;c++) state[r][c]=tmp[c];
        }
        for(int c=0;c<4;c++){
            int wk = roundKeyWords[48 + c];
            for(int r=0;r<4;r++){
                int byte = (wk >> (24-8*r)) & 0xFF;
                state[r][c] ^= byte;
            }
        }

        /* Store ciphertext (column‑major) */
        for(int c=0;c<4;c++) for(int r=0;r<4;r++) outBytes[4*c+r]=state[r][c] & 0xFF;
    };

    /*--- Process vectors with mixed signed extremes -----------------*/
    int rcw[4*13];               // round keys storage (13×4 words)
    int ct[16];                  // ciphertext buffer

    // Test 1: mixed key / zero plaintext
    expandKey(keyMixed, rcw);
    encryptBlock(ptZero, ct, rcw);
    std::cout << "Mixed‑key / Zero‑plain : ";
    for(int i=0;i<16;i++) std::cout << std::hex << std::setw(2) << std::setfill('0') << ct[i];
    std::cout << std::dec << std::endl;

    // Test 2: full‑FF key / mixed signed plaintext
    expandKey(keyFull, rcw);
    encryptBlock(ptMixed, ct, rcw);
    std::cout << "Full‑key / Mixed‑plain : ";
    for(int i=0;i<16;i++) std::cout << std::hex << std::setw(2) << std::setfill('0') << ct[i];
    std::cout << std::dec << std::endl;

    return 0;
}
