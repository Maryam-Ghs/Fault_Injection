#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;

/* LLM input variant 10: large-safe-stress */

/* ---------- S‑Box (plain int, no const) ---------- */
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

/* ---------- Rcon (plain int) ---------- */
int rcon[10] = {1,2,4,8,16,32,64,128,27,54};

/* ---------- Helper: xtime for MixColumns ---------- */
int xtime(int x) {
    int y = (x << 1) & 0xFF;
    if (x & 0x80) y ^= 0x1B;
    return y;
}

/* ---------- SubBytes ---------- */
void sub_bytes(int state[4][4]) {
    int i = 0, j = 0;
    while (i < 4) {
        j = 0;
        while (j < 4) {
            int idx = state[i][j];
            state[i][j] = sbox[idx];
            j = j + 1;
        }
        i = i + 1;
    }
}

/* ---------- ShiftRows (left cyclic) ---------- */
void shift_rows(int state[4][4]) {
    int r = 1;
    while (r < 4) {
        int tmp0 = state[r][0];
        int tmp1 = state[r][1];
        int tmp2 = state[r][2];
        int tmp3 = state[r][3];
        if (r == 1) {
            state[r][0] = tmp1; state[r][1] = tmp2; state[r][2] = tmp3; state[r][3] = tmp0;
        } else if (r == 2) {
            state[r][0] = tmp2; state[r][1] = tmp3; state[r][2] = tmp0; state[r][3] = tmp1;
        } else { /* r == 3 */
            state[r][0] = tmp3; state[r][1] = tmp0; state[r][2] = tmp1; state[r][3] = tmp2;
        }
        r = r + 1;
    }
}

/* ---------- MixColumns (using temporary vars) ---------- */
void mix_columns(int state[4][4]) {
    int c = 0;
    while (c < 4) {
        int a0 = state[0][c];
        int a1 = state[1][c];
        int a2 = state[2][c];
        int a3 = state[3][c];

        int t = a0 ^ a1 ^ a2 ^ a3;

        int u0 = a0;
        int u1 = a1;
        int u2 = a2;
        int u3 = a3;

        int m0 = a0 ^ t ^ xtime(a0 ^ a1);
        int m1 = a1 ^ t ^ xtime(a1 ^ a2);
        int m2 = a2 ^ t ^ xtime(a2 ^ a3);
        int m3 = a3 ^ t ^ xtime(a3 ^ u0);

        state[0][c] = m0;
        state[1][c] = m1;
        state[2][c] = m2;
        state[3][c] = m3;

        c = c + 1;
    }
}

/* ---------- AddRoundKey ---------- */
void add_round_key(int state[4][4], int roundKey[][4], int round) {
    int col = 0;
    while (col < 4) {
        int row = 0;
        while (row < 4) {
            state[row][col] ^= roundKey[round * 4 + col][row];
            row = row + 1;
        }
        col = col + 1;
    }
}

/* ---------- Key Expansion for AES‑192 ---------- */
void expand_key(int key[24], int roundKey[52][4]) {
    int i = 0;
    while (i < 6) { /* first 6 words directly from key */
        roundKey[i][0] = key[4 * i + 0];
        roundKey[i][1] = key[4 * i + 1];
        roundKey[i][2] = key[4 * i + 2];
        roundKey[i][3] = key[4 * i + 3];
        i = i + 1;
    }

    int n = 6;
    int r = 0;
    while (n < 52) {
        int temp[4];
        temp[0] = roundKey[n - 1][0];
        temp[1] = roundKey[n - 1][1];
        temp[2] = roundKey[n - 1][2];
        temp[3] = roundKey[n - 1][3];

        if (n % 6 == 0) {
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
            temp[0] ^= rcon[r];
            r = r + 1;
        } else if (n % 6 == 4) {
            /* SubWord only */
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
        }

        roundKey[n][0] = roundKey[n - 6][0] ^ temp[0];
        roundKey[n][1] = roundKey[n - 6][1] ^ temp[1];
        roundKey[n][2] = roundKey[n - 6][2] ^ temp[2];
        roundKey[n][3] = roundKey[n - 6][3] ^ temp[3];
        n = n + 1;
    }
}

/* ---------- Encryption of a single 16‑byte block ---------- */
void encrypt_block(int plain[16], int key[24], int cipher[16]) {
    int state[4][4];
    int col = 0;
    while (col < 4) {
        int row = 0;
        while (row < 4) {
            state[row][col] = plain[col * 4 + row];
            row = row + 1;
        }
        col = col + 1;
    }

    int roundKey[52][4];
    expand_key(key, roundKey);

    add_round_key(state, roundKey, 0);

    int round = 1;
    while (round <= 11) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, roundKey, round);
        round = round + 1;
    }

    /* final round (no MixColumns) */
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, roundKey, 12);

    col = 0;
    while (col < 4) {
        int row = 0;
        while (row < 4) {
            cipher[col * 4 + row] = state[row][col];
            row = row + 1;
        }
        col = col + 1;
    }
}

/* ---------- Helper: print array as hex ---------- */
void print_hex(const char* title, int arr[], int len) {
    cout << title;
    int i = 0;
    while (i < len) {
        cout << setfill('0') << setw(2) << hex << (arr[i] & 0xFF);
        if (i + 1 < len) cout << " ";
        i = i + 1;
    }
    cout << dec << endl;
}

/* ---------- Main ---------- */
int main() {
    const int NUM_BLOCKS = 1000;          // large but safe number of blocks
    const int KEY_SIZE   = 24;            // 192‑bit key (24 bytes)
    const int BLOCK_SIZE = 16;            // AES block size

    /* deterministic 192‑bit key: 0,1,2,...,23 */
    int arrKey[KEY_SIZE];
    for (int i = 0; i < KEY_SIZE; ++i) {
        arrKey[i] = i & 0xFF;
    }

    /* allocate plaintext and ciphertext buffers for all blocks */
    int *arrPlain  = new int[NUM_BLOCKS * BLOCK_SIZE];
    int *arrCipher = new int[NUM_BLOCKS * BLOCK_SIZE];

    /* fill plaintext with a simple pattern: each byte = (block_index*BLOCK_SIZE + offset) % 256 */
    for (int b = 0; b < NUM_BLOCKS; ++b) {
        for (int off = 0; off < BLOCK_SIZE; ++off) {
            arrPlain[b * BLOCK_SIZE + off] = (b * BLOCK_SIZE + off) & 0xFF;
        }
    }

    /* encrypt each block */
    for (int b = 0; b < NUM_BLOCKS; ++b) {
        encrypt_block(&arrPlain[b * BLOCK_SIZE], arrKey, &arrCipher[b * BLOCK_SIZE]);
    }

    /* print first 5 plaintext/ciphertext pairs to illustrate correctness */
    int displayCount = (NUM_BLOCKS < 5) ? NUM_BLOCKS : 5;
    for (int b = 0; b < displayCount; ++b) {
        cout << "Block " << b << " Plaintext : ";
        print_hex("", &arrPlain[b * BLOCK_SIZE], BLOCK_SIZE);
        cout << "Block " << b << " Ciphertext: ";
        print_hex("", &arrCipher[b * BLOCK_SIZE], BLOCK_SIZE);
        cout << endl;
    }

    /* print the deterministic key once */
    print_hex("Key       : ", arrKey, KEY_SIZE);

    delete[] arrPlain;
    delete[] arrCipher;
    return 0;
}
