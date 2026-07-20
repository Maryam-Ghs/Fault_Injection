/******************************************************************
 * AES‑128‑XTS implementation – version #7
 * --------------------------------------------------------------
 *  * Only int / float types are used (no unsigned, long, double,
 *    const, etc.)
 *  * Input data are small predefined arrays generated inside main()
 *  * All buffers are allocated on the stack
 *  * Functions are used for modularity, but every variable lives in
 *    main (except function parameters)
 *  * Math is split into temporary variables
 *  * Loop structures and variable names differ from earlier versions
 ******************************************************************/

/* LLM input variant 8: sparse-skewed */

#include <iostream>
#include <iomanip>

/* ---------- S‑box ------------------------------------------------ */
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

/* ---------- Rcon ------------------------------------------------- */
int rcon[11] = {0x00000000,0x01000000,0x02000000,0x04000000,
                0x08000000,0x10000000,0x20000000,0x40000000,
                0x80000000,0x1B000000,0x36000000};

/* ---------- Helper: multiply in GF(2^8) -------------------------- */
int xtime(int x) {
    int hi = (x & 0x80);
    int res = (x << 1) & 0xFF;
    if (hi) res ^= 0x1B;
    return res;
}

/* ---------- Key schedule ------------------------------------------ */
void expandKey(const int key[16], int roundKey[44][4]) {
    int i = 0;
    while (i < 4) {
        roundKey[i][0] = key[4*i];
        roundKey[i][1] = key[4*i+1];
        roundKey[i][2] = key[4*i+2];
        roundKey[i][3] = key[4*i+3];
        ++i;
    }
    i = 4;
    while (i < 44) {
        int temp[4];
        temp[0] = roundKey[i-1][0];
        temp[1] = roundKey[i-1][1];
        temp[2] = roundKey[i-1][2];
        temp[3] = roundKey[i-1][3];

        if (i % 4 == 0) {
            /* RotWord */
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            /* SubWord */
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
            /* Rcon */
            int rc = (rcon[i/4] >> 24) & 0xFF;
            temp[0] ^= rc;
        }

        roundKey[i][0] = roundKey[i-4][0] ^ temp[0];
        roundKey[i][1] = roundKey[i-4][1] ^ temp[1];
        roundKey[i][2] = roundKey[i-4][2] ^ temp[2];
        roundKey[i][3] = roundKey[i-4][3] ^ temp[3];
        ++i;
    }
}

/* ---------- Core AES steps --------------------------------------- */
void subBytes(int state[4][4]) {
    int r = 0;
    while (r < 4) {
        int c = 0;
        while (c < 4) {
            state[r][c] = sbox[state[r][c]];
            ++c;
        }
        ++r;
    }
}

void shiftRows(int state[4][4]) {
    /* Row 1 */
    int t = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = t;

    /* Row 2 (swap) */
    int t0 = state[2][0];
    int t1 = state[2][1];
    state[2][0] = state[2][2];
    state[2][1] = state[2][3];
    state[2][2] = t0;
    state[2][3] = t1;

    /* Row 3 */
    t = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = t;
}

void mixColumns(int state[4][4]) {
    int c = 0;
    while (c < 4) {
        int a0 = state[0][c];
        int a1 = state[1][c];
        int a2 = state[2][c];
        int a3 = state[3][c];

        int m0 = xtime(a0) ^ xtime(a1) ^ a1 ^ a2 ^ a3;
        int m1 = a0 ^ xtime(a1) ^ xtime(a2) ^ a2 ^ a3;
        int m2 = a0 ^ a1 ^ xtime(a2) ^ xtime(a3) ^ a3;
        int m3 = xtime(a0) ^ a0 ^ a1 ^ a2 ^ xtime(a3);

        state[0][c] = m0 & 0xFF;
        state[1][c] = m1 & 0xFF;
        state[2][c] = m2 & 0xFF;
        state[3][c] = m3 & 0xFF;
        ++c;
    }
}

void addRoundKey(int state[4][4], const int roundKey[4][4]) {
    int r = 0;
    while (r < 4) {
        int c = 0;
        while (c < 4) {
            int rkByte = (roundKey[c][r>>2] >> ((r & 3)*8)) & 0xFF;
            state[r][c] ^= rkByte;
            ++c;
        }
        ++r;
    }
}

/* ---------- AES‑128 block encryption ------------------------------ */
void aesEncryptBlock(const int in[16], int out[16], const int key[16]) {
    int state[4][4];
    /* load input into state (column‑major) */
    int i = 0;
    while (i < 16) {
        int row = i % 4;
        int col = i / 4;
        state[row][col] = in[i] & 0xFF;
        ++i;
    }

    int roundKey[44][4];
    expandKey(key, roundKey);

    addRoundKey(state, (int (*)[4])&roundKey[0]);

    int round = 1;
    while (round < 10) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, (int (*)[4])&roundKey[round*4]);
        ++round;
    }

    subBytes(state);
    shiftRows(state);
    addRoundKey(state, (int (*)[4])&roundKey[40]);

    /* store state to output */
    i = 0;
    while (i < 16) {
        int row = i % 4;
        int col = i / 4;
        out[i] = state[row][col] & 0xFF;
        ++i;
    }
}

/* ---------- GF(2^128) multiplication by α (x) -------------------- */
void mulAlpha(int tweak[16]) {
    int carry = 0;
    int idx = 0;
    while (idx < 16) {
        int nextCarry = (tweak[idx] & 0x80) ? 1 : 0;
        tweak[idx] = ((tweak[idx] << 1) & 0xFF) | carry;
        carry = nextCarry;
        ++idx;
    }
    if (carry) tweak[0] ^= 0x87;   // reduction polynomial
}

/* ---------- XTS single‑block encrypt ----------------------------- */
void xtsEncryptBlock(const int plain[16], int cipher[16],
                     const int keyData[16], const int keyTweak[16],
                     const int iv[16]) {
    int tweak[16];
    aesEncryptBlock(iv, tweak, keyTweak);      // compute initial tweak

    int tmp[16];
    int i = 0;
    while (i < 16) {
        tmp[i] = plain[i] ^ tweak[i];
        ++i;
    }

    aesEncryptBlock(tmp, tmp, keyData);

    i = 0;
    while (i < 16) {
        cipher[i] = tmp[i] ^ tweak[i];
        ++i;
    }

    mulAlpha(tweak);   // next tweak (not used further here)
}

/* ---------- Main ------------------------------------------------- */
int main() {
    /* --- sparse‑skewed predefined plaintext, keys and IV --------- */
    int plaintext[16] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff
    };
    int keyData[16] = {
        0x2b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x15,0x00,0x00,0x00,0x4f,0x00
    };
    int keyTweak[16] = {
        0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x0a,0x00,0x00,0x00,0x00,0x0f
    };
    int iv[16] = {               // sector number / tweak input (sparse)
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };

    int ciphertext[16];
    xtsEncryptBlock(plaintext, ciphertext,
                    keyData, keyTweak, iv);

    /* ----- display results --------------------------------------- */
    std::cout << "Plaintext : ";
    int i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (plaintext[i] & 0xFF) << ' ';
        ++i;
    }
    std::cout << "\nCiphertext: ";
    i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (ciphertext[i] & 0xFF) << ' ';
        ++i;
    }
    std::cout << std::dec << std::endl;
    return 0;
}
