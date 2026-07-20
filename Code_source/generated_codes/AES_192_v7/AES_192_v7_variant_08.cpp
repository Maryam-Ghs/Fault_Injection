/*********************************************************************
 *  AES‑192 implementation – Version #7
 *  Constraints:
 *   - Only int and float types are used (no double, long, unsigned, const)
 *   - All data allocated on the heap (new/delete)
 *   - No std::cin – inputs are generated internally
 *   - Results are printed to stdout
 *   - Heavy use of loops, reordered arithmetic, and helper functions
 *********************************************************************/

#include <cstdio>
#include <cstdlib>
#include <ctime>

/* LLM input variant 8: sparse-skewed */

/* ----------------------------------------------------------------- */
/*  S‑box (fixed 256‑byte substitution table)                         */
static int sbox[256] = {
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

/* ----------------------------------------------------------------- */
/*  Round constants (Rcon) – only the first 12 are needed for AES‑192 */
static int rcon[12] = {1,2,4,8,16,32,64,128,27,54,108,216};

/* ----------------------------------------------------------------- */
/*  Helper: rotate a 4‑byte word left by one byte                       */
void rotWord(int *w)
{
    int tmp = w[0];
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = tmp;
}

/* ----------------------------------------------------------------- */
/*  Helper: apply S‑box to each byte of a word                        */
void subWord(int *w)
{
    int i = 0;
    while (i < 4) {
        w[i] = sbox[w[i]];
        i = i + 1;
    }
}

/* ----------------------------------------------------------------- */
/*  Key expansion – produces 52 4‑byte words (208 bytes)               */
void expandKey(int *keyBytes, int *expanded)
{
    int i = 0;
    /* copy the original 24‑byte key (6 words) */
    while (i < 24) {
        expanded[i] = keyBytes[i];
        i = i + 1;
    }

    int wordsNeeded = 52;               /* 4*(Nr+1) = 4*13 */
    int curWord = 6;                    /* Nk = 6 */
    int temp[4];

    while (curWord < wordsNeeded) {
        /* copy previous word into temp */
        int j = 0;
        while (j < 4) {
            temp[j] = expanded[(curWord - 1) * 4 + j];
            j = j + 1;
        }

        if (curWord % 6 == 0) {
            rotWord(temp);
            subWord(temp);
            temp[0] = temp[0] ^ rcon[(curWord / 6) - 1];
        } else if (curWord % 6 == 4) {
            subWord(temp);
        }

        /* XOR with the word Nk positions earlier */
        j = 0;
        while (j < 4) {
            expanded[curWord * 4 + j] = expanded[(curWord - 6) * 4 + j] ^ temp[j];
            j = j + 1;
        }
        curWord = curWord + 1;
    }
}

/* ----------------------------------------------------------------- */
/*  AddRoundKey – XOR state with round key                              */
void addRoundKey(int *state, int *roundKey)
{
    int i = 0;
    while (i < 16) {
        state[i] = state[i] ^ roundKey[i];
        i = i + 1;
    }
}

/* ----------------------------------------------------------------- */
/*  SubBytes – byte‑wise substitution using the S‑box                  */
void subBytes(int *state)
{
    int i = 0;
    while (i < 16) {
        state[i] = sbox[state[i]];
        i = i + 1;
    }
}

/* ----------------------------------------------------------------- */
/*  ShiftRows – cyclic shift of rows                                   */
void shiftRows(int *state)
{
    int tmp;

    /* Row 1 – shift left by 1 */
    tmp = state[1];
    state[1]  = state[5];
    state[5]  = state[9];
    state[9]  = state[13];
    state[13] = tmp;

    /* Row 2 – shift left by 2 */
    tmp = state[2];
    state[2]  = state[10];
    state[10] = tmp;
    tmp = state[6];
    state[6]  = state[14];
    state[14] = tmp;

    /* Row 3 – shift left by 3 (right by 1) */
    tmp = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7]  = state[3];
    state[3]  = tmp;
}

/* ----------------------------------------------------------------- */
/*  MixColumns – matrix multiplication in GF(2^8)                     */
static int xtime(int x)
{
    int shifted = x << 1;
    if (x & 0x80) shifted = shifted ^ 0x1b;
    return shifted & 0xff;
}

void mixColumns(int *state)
{
    int c = 0;
    while (c < 4) {
        int i0 = c * 4;
        int a0 = state[i0];
        int a1 = state[i0 + 1];
        int a2 = state[i0 + 2];
        int a3 = state[i0 + 3];

        int t  = a0 ^ a1 ^ a2 ^ a3;
        int u0 = a0;
        int u1 = a1;
        int u2 = a2;
        int u3 = a3;

        /* Reordered arithmetic – compute each column byte */
        state[i0]     = a0 ^ t ^ xtime(a0 ^ a1);
        state[i0 + 1] = a1 ^ t ^ xtime(a1 ^ a2);
        state[i0 + 2] = a2 ^ t ^ xtime(a2 ^ a3);
        state[i0 + 3] = a3 ^ t ^ xtime(a3 ^ u0);

        c = c + 1;
    }
}

/* ----------------------------------------------------------------- */
/*  Encrypt a single 16‑byte block                                     */
void encryptBlock(int *in, int *out, int *roundKeys)
{
    int *state = new int[16];
    int i = 0;
    while (i < 16) {
        state[i] = in[i];
        i = i + 1;
    }

    /* Initial round */
    addRoundKey(state, roundKeys);

    int round = 1;
    while (round < 12) {                 /* Nr = 12 */
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, roundKeys + round * 16);
        round = round + 1;
    }

    /* Final round (no MixColumns) */
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, roundKeys + 12 * 16);

    i = 0;
    while (i < 16) {
        out[i] = state[i];
        i = i + 1;
    }
    delete[] state;
}

/* ----------------------------------------------------------------- */
int main()
{
    std::srand((unsigned)std::time(0));

    /* -----------------------------------------------------------------
       Generate deterministic sparse‑skewed plaintext (16 bytes) and
       key (24 bytes). Most bytes are zero; a few non‑zero values are
       clustered near the beginning and end.
       ----------------------------------------------------------------- */
    int *plain = new int[16];
    int *key   = new int[24];
    int i = 0;

    /* Initialize plaintext to zeros */
    while (i < 16) {
        plain[i] = 0;
        i = i + 1;
    }
    /* Sparse non‑zero cluster */
    plain[0] = 0x3a;
    plain[1] = 0x7f;
    plain[14] = 0x55;
    plain[15] = 0x99;

    i = 0;
    /* Initialize key to zeros */
    while (i < 24) {
        key[i] = 0;
        i = i + 1;
    }
    /* Sparse non‑zero cluster in key */
    key[0] = 0x2b;
    key[1] = 0x1c;
    key[2] = 0x00;   // explicit zero for clarity
    key[22] = 0x88;
    key[23] = 0xff;

    /* -----------------------------------------------------------------
       Expand the key
       ----------------------------------------------------------------- */
    int *roundKeys = new int[208];      /* 52 words * 4 bytes */
    expandKey(key, roundKeys);

    /* -----------------------------------------------------------------
       Encrypt
       ----------------------------------------------------------------- */
    int *cipher = new int[16];
    encryptBlock(plain, cipher, roundKeys);

    /* -----------------------------------------------------------------
       Print results in hex
       ----------------------------------------------------------------- */
    std::printf("Plaintext : ");
    i = 0;
    while (i < 16) {
        std::printf("%02x ", plain[i]);
        i = i + 1;
    }
    std::printf("\nKey       : ");
    i = 0;
    while (i < 24) {
        std::printf("%02x ", key[i]);
        i = i + 1;
    }
    std::printf("\nCiphertext: ");
    i = 0;
    while (i < 16) {
        std::printf("%02x ", cipher[i]);
        i = i + 1;
    }
    std::printf("\n");

    /* Clean up */
    delete[] plain;
    delete[] key;
    delete[] roundKeys;
    delete[] cipher;
    return 0;
}
