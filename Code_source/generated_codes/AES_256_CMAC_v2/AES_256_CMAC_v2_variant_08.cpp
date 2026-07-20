#include <iostream>

/* LLM input variant 8: sparse-skewed */

int main() {
    /*--------------------------------------------------------------
       AES‑256 S‑box (plain int array, no const)
    --------------------------------------------------------------*/
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

    /*--------------------------------------------------------------
       Rcon for key expansion (int array)
    --------------------------------------------------------------*/
    int rcon[15] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
                    0x6C,0xD8,0xAB,0x4D,0x9A};

    /*--------------------------------------------------------------
       Test vector – 256‑bit key (sparse, clustered)
    --------------------------------------------------------------*/
    int key[32] = {
        0x00,0x00,0x00,0x01,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x02,
        0x00,0x00,0x00,0x00
    };

    /*--------------------------------------------------------------
       Test message – 16 bytes (mostly zeros, single non‑zero at end)
    --------------------------------------------------------------*/
    int msg[16] = {
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0xFF
    };
    int msgLen = 16;                     /* length in bytes */

    /*--------------------------------------------------------------
       Key expansion – produce 60 words (4‑byte each)
    --------------------------------------------------------------*/
    int w[60][4];
    int i,j,t;

    /* copy the original key into the first 8 words */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++) {
            w[i][j] = key[4*i + j];
        }
    }

    /* generate the remaining words */
    for (i = 8; i < 60; i++) {
        /* copy previous word to temporary buffer */
        for (j = 0; j < 4; j++) {
            t = w[i-1][j];
        }

        if (i % 8 == 0) {                     /* RotWord + SubWord + Rcon */
            /* RotWord */
            int tmp = w[i-1][0];
            for (j = 0; j < 3; j++) {
                w[i-1][j] = w[i-1][j+1];
            }
            w[i-1][3] = tmp;
            /* SubWord */
            for (j = 0; j < 4; j++) {
                w[i-1][j] = sbox[w[i-1][j]];
            }
            /* Rcon on the first byte */
            w[i-1][0] ^= rcon[i/8];
        } else if (i % 8 == 4) {              /* SubWord only */
            for (j = 0; j < 4; j++) {
                w[i-1][j] = sbox[w[i-1][j]];
            }
        }

        /* w[i] = w[i-8] xor w[i-1] (after possible transformation) */
        for (j = 0; j < 4; j++) {
            w[i][j] = (w[i-8][j] ^ w[i-1][j]) & 0xFF;
        }
    }

    /*--------------------------------------------------------------
       Build round keys – 15 round keys (0 … 14) each 16 bytes
    --------------------------------------------------------------*/
    int roundKey[15][16];
    for (i = 0; i <= 14; i++) {
        for (j = 0; j < 4; j++) {            /* 4 words per round key */
            int wordIdx = i*4 + j;
            roundKey[i][4*j + 0] = w[wordIdx][0];
            roundKey[i][4*j + 1] = w[wordIdx][1];
            roundKey[i][4*j + 2] = w[wordIdx][2];
            roundKey[i][4*j + 3] = w[wordIdx][3];
        }
    }

    /*--------------------------------------------------------------
       Helper lambdas (inline inside main, using only int)
    --------------------------------------------------------------*/
    auto xtime = [&](int x)->int{
        int shifted = (x << 1) & 0xFF;
        if (x & 0x80) shifted ^= 0x1B;
        return shifted & 0xFF;
    };

    /*--------------------------------------------------------------
       AES‑256 encryption of a single 16‑byte block (inPlace)
    --------------------------------------------------------------*/
    auto aesEncrypt = [&](int in[16], int out[16]){
        int state[4][4];                     /* row, column */
        int r,c;

        /* copy input (column‑major) */
        for (c = 0; c < 4; c++) {
            for (r = 0; r < 4; r++) {
                state[r][c] = in[4*c + r] & 0xFF;
            }
        }

        /* AddRoundKey – round 0 */
        for (c = 0; c < 4; c++) {
            for (r = 0; r < 4; r++) {
                state[r][c] ^= roundKey[0][4*c + r];
            }
        }

        /* 13 full rounds (1 … 13) */
        for (i = 1; i <= 13; i++) {
            /* SubBytes */
            for (r = 0; r < 4; r++) {
                for (c = 0; c < 4; c++) {
                    state[r][c] = sbox[state[r][c]];
                }
            }
            /* ShiftRows */
            int tmp;
            /* row 1 */
            tmp = state[1][0];
            state[1][0] = state[1][1];
            state[1][1] = state[1][2];
            state[1][2] = state[1][3];
            state[1][3] = tmp;
            /* row 2 (rotate 2) */
            tmp = state[2][0];
            state[2][0] = state[2][2];
            state[2][2] = tmp;
            tmp = state[2][1];
            state[2][1] = state[2][3];
            state[2][3] = tmp;
            /* row 3 (rotate 3) */
            tmp = state[3][3];
            state[3][3] = state[3][2];
            state[3][2] = state[3][1];
            state[3][1] = state[3][0];
            state[3][0] = tmp;

            /* MixColumns */
            for (c = 0; c < 4; c++) {
                int a0 = state[0][c];
                int a1 = state[1][c];
                int a2 = state[2][c];
                int a3 = state[3][c];
                int t0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
                int t1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
                int t2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
                int t3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
                state[0][c] = t0 & 0xFF;
                state[1][c] = t1 & 0xFF;
                state[2][c] = t2 & 0xFF;
                state[3][c] = t3 & 0xFF;
            }

            /* AddRoundKey */
            for (c = 0; c < 4; c++) {
                for (r = 0; r < 4; r++) {
                    state[r][c] ^= roundKey[i][4*c + r];
                }
            }
        }

        /* Final round (no MixColumns) */
        /* SubBytes */
        for (r = 0; r < 4; r++) {
            for (c = 0; c < 4; c++) {
                state[r][c] = sbox[state[r][c]];
            }
        }
        /* ShiftRows */
        int tmp;
        /* row 1 */
        tmp = state[1][0];
        state[1][0] = state[1][1];
        state[1][1] = state[1][2];
        state[1][2] = state[1][3];
        state[1][3] = tmp;
        /* row 2 */
        tmp = state[2][0];
        state[2][0] = state[2][2];
        state[2][2] = tmp;
        tmp = state[2][1];
        state[2][1] = state[2][3];
        state[2][3] = tmp;
        /* row 3 */
        tmp = state[3][3];
        state[3][3] = state[3][2];
        state[3][2] = state[3][1];
        state[3][1] = state[3][0];
        state[3][0] = tmp;

        /* AddRoundKey – round 14 */
        for (c = 0; c < 4; c++) {
            for (r = 0; r < 4; r++) {
                state[r][c] ^= roundKey[14][4*c + r];
            }
        }

        /* copy state back to out (column‑major) */
        for (c = 0; c < 4; c++) {
            for (r = 0; r < 4; r++) {
                out[4*c + r] = state[r][c] & 0xFF;
            }
        }
    };

    /*--------------------------------------------------------------
       Subkey generation for CMAC (K1, K2)
    --------------------------------------------------------------*/
    int zeroBlock[16] = {0};
    int L[16];
    aesEncrypt(zeroBlock, L);

    auto leftShiftOne = [&](int in[16], int out[16]){
        int carry = 0, nextCarry;
        for (i = 15; i >= 0; i--) {
            nextCarry = (in[i] & 0x80) ? 1 : 0;
            out[i] = ((in[i] << 1) & 0xFF) | carry;
            carry = nextCarry;
        }
    };

    int K1[16], K2[16];
    leftShiftOne(L, K1);
    if (L[0] & 0x80) {                     /* MSB = 1 */
        K1[15] ^= 0x87;
    }
    leftShiftOne(K1, K2);
    if (K1[0] & 0x80) {
        K2[15] ^= 0x87;
    }

    /*--------------------------------------------------------------
       CMAC processing
    --------------------------------------------------------------*/
    int blockCount = (msgLen + 15) / 16;    /* ceil */
    int lastBlockIsComplete = (msgLen % 16 == 0 && msgLen != 0) ? 1 : 0;

    int X[16] = {0};                       /* chaining variable */
    int M_i[16];
    int Y[16];

    for (i = 0; i < blockCount - 1; i++) {
        for (j = 0; j < 16; j++) {
            M_i[j] = msg[16*i + j];
            X[j] ^= M_i[j];
        }
        aesEncrypt(X, X);                  /* X = AES(K, X) */
    }

    /* build the last block */
    int last[16];
    if (lastBlockIsComplete) {
        for (j = 0; j < 16; j++) {
            last[j] = msg[16*(blockCount-1) + j] ^ K1[j];
        }
    } else {
        int padLen = 16 - (msgLen % 16);
        for (j = 0; j < 16; j++) {
            if (j < (msgLen % 16))
                last[j] = msg[16*(blockCount-1) + j];
            else if (j == (msgLen % 16))
                last[j] = 0x80;
            else
                last[j] = 0x00;
            last[j] ^= K2[j];
        }
    }

    for (j = 0; j < 16; j++) {
        X[j] ^= last[j];
    }
    aesEncrypt(X, Y);                      /* final AES */

    /*--------------------------------------------------------------
       Output CMAC (hex)
    --------------------------------------------------------------*/
    std::cout << "CMAC = ";
    for (i = 0; i < 16; i++) {
        int hi = (Y[i] >> 4) & 0xF;
        int lo = Y[i] & 0xF;
        std::cout << (char)('0' + (hi < 10 ? hi : hi - 10 + 'a')) 
                  << (char)('0' + (lo < 10 ? lo : lo - 10 + 'a'));
    }
    std::cout << std::endl;
    return 0;
}
