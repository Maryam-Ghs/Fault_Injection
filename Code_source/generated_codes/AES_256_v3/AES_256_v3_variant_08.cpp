#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;

/* LLM input variant 8: sparse-skewed */

/* -------------------------------------------------------------
   AES‑256 implementation – only int types, heap allocation,
   class based, expanded step‑by‑step operations.
   ------------------------------------------------------------- */

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

static int rcon[15] = {
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
    0x6C,0xD8,0xAB,0x4D,0x9A
};

/* ------------------------------------------------------------- */

class AES256 {
public:
    int *roundKey;                 // Expanded key (240 bytes)

    AES256(int *rawKey) {
        int keyBytes = 32;        // 256‑bit key = 32 bytes
        roundKey = new int[240];  // 60 words * 4 bytes
        keyExpansion(rawKey, keyBytes);
    }

    ~AES256() {
        delete [] roundKey;
    }

    void encryptBlock(int *plainBlock, int *cipherBlock) {
        int *state = new int[16];
        int i = 0;
        while (i < 16) {
            state[i] = plainBlock[i];
            i = i + 1;
        }

        addRoundKey(state, 0);

        int round = 1;
        while (round < 14) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
            round = round + 1;
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 14);

        i = 0;
        while (i < 16) {
            cipherBlock[i] = state[i];
            i = i + 1;
        }

        delete [] state;
    }

private:
    /* ----- Key Expansion -------------------------------------- */
    void keyExpansion(int *key, int keyLen) {
        int Nk = 8;                 // 8 words in 256‑bit key
        int Nb = 4;
        int Nr = 14;
        int totalWords = Nb * (Nr + 1);   // 60

        int i = 0;
        while (i < keyLen) {
            roundKey[i] = key[i];
            i = i + 1;
        }

        int bytesGenerated = keyLen;
        int temp[4];

        while (bytesGenerated < totalWords * 4) {
            /* ---- copy previous word ---- */
            int j = 0;
            while (j < 4) {
                temp[j] = roundKey[bytesGenerated - 4 + j];
                j = j + 1;
            }

            /* ---- apply core transformation ---- */
            if ((bytesGenerated / 4) % Nk == 0) {
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
                temp[0] = temp[0] ^ rcon[(bytesGenerated / 4) / Nk];
            } else if (Nk > 6 && ((bytesGenerated / 4) % Nk) == 4) {
                /* SubWord only */
                temp[0] = sbox[temp[0]];
                temp[1] = sbox[temp[1]];
                temp[2] = sbox[temp[2]];
                temp[3] = sbox[temp[3]];
            }

            /* ---- XOR with word Nk positions earlier ---- */
            j = 0;
            while (j < 4) {
                roundKey[bytesGenerated] = roundKey[bytesGenerated - Nk * 4] ^ temp[j];
                roundKey[bytesGenerated] = roundKey[bytesGenerated] & 0xFF;
                bytesGenerated = bytesGenerated + 1;
                j = j + 1;
            }
        }
    }

    /* ----- Core AES Transformations --------------------------- */
    void addRoundKey(int *state, int round) {
        int i = 0;
        while (i < 16) {
            state[i] = state[i] ^ roundKey[round * 16 + i];
            state[i] = state[i] & 0xFF;
            i = i + 1;
        }
    }

    void subBytes(int *state) {
        int i = 0;
        while (i < 16) {
            state[i] = sbox[state[i]];
            i = i + 1;
        }
    }

    void shiftRows(int *state) {
        int temp;

        /* Row 1 : shift left by 1 */
        temp = state[1];
        state[1]  = state[5];
        state[5]  = state[9];
        state[9]  = state[13];
        state[13] = temp;

        /* Row 2 : shift left by 2 */
        temp = state[2];
        state[2]  = state[10];
        state[10] = temp;
        temp = state[6];
        state[6]  = state[14];
        state[14] = temp;

        /* Row 3 : shift left by 3 (right by 1) */
        temp = state[3];
        state[3]  = state[15];
        state[15] = state[11];
        state[11] = state[7];
        state[7]  = temp;
    }

    int xtime(int x) {
        int shifted = (x << 1) & 0xFF;
        if (x & 0x80) shifted = shifted ^ 0x1B;
        return shifted;
    }

    void mixColumns(int *state) {
        int c = 0;
        while (c < 4) {
            int i0 = 4 * c;
            int a0 = state[i0];
            int a1 = state[i0 + 1];
            int a2 = state[i0 + 2];
            int a3 = state[i0 + 3];

            int t = a0 ^ a1 ^ a2 ^ a3;
            int u = a0;

            state[i0]     = a0 ^ t ^ xtime(a0 ^ a1);
            state[i0 + 1] = a1 ^ t ^ xtime(a1 ^ a2);
            state[i0 + 2] = a2 ^ t ^ xtime(a2 ^ a3);
            state[i0 + 3] = a3 ^ t ^ xtime(a3 ^ u);

            c = c + 1;
        }
    }
};

/* ------------------------------------------------------------- */

int main() {
    /* ---- deterministic sparse 256‑bit key ---- */
    int *keyBytes = new int[32];
    for (int i = 0; i < 32; ++i) keyBytes[i] = 0;
    // a few non‑zero bytes (clustered at the start)
    keyBytes[0] = 0x2b;
    keyBytes[1] = 0x7e;
    keyBytes[2] = 0x15;
    keyBytes[3] = 0x16;
    keyBytes[4] = 0x28;
    keyBytes[5] = 0xae;
    keyBytes[6] = 0xd2;
    keyBytes[7] = 0xa6;

    /* ---- deterministic sparse plaintext blocks ---- */
    int blockCount = 10;                 // keep number modest
    int *plainData = new int[blockCount * 16];
    for (int i = 0; i < blockCount * 16; ++i) plainData[i] = 0;

    // Populate a few blocks with clustered non‑zero values
    // Block 0: a typical test vector pattern
    plainData[0]  = 0x32; plainData[1]  = 0x43; plainData[2]  = 0xf6; plainData[3]  = 0xa8;
    plainData[4]  = 0x88; plainData[5]  = 0x5a; plainData[6]  = 0x30; plainData[7]  = 0x8d;
    plainData[8]  = 0x31; plainData[9]  = 0x31; plainData[10] = 0x98; plainData[11] = 0xa2;
    plainData[12] = 0xe0; plainData[13] = 0x37; plainData[14] = 0x07; plainData[15] = 0x34;

    // Block 5: a small sparse burst
    int base5 = 5 * 16;
    plainData[base5 + 2] = 0xff;
    plainData[base5 + 7] = 0x01;
    plainData[base5 + 12] = 0x80;

    // Block 8: another tiny burst
    int base8 = 8 * 16;
    plainData[base8 + 0] = 0x55;
    plainData[base8 + 15] = 0xaa;

    int *cipherData = new int[blockCount * 16];

    AES256 aesEngine(keyBytes);

    int blk = 0;
    while (blk < blockCount) {
        aesEngine.encryptBlock(&plainData[blk * 16], &cipherData[blk * 16]);
        blk = blk + 1;
    }

    /* ---- print results ---- */
    cout << "AES‑256 encryption of " << blockCount << " sparse blocks\n";
    blk = 0;
    while (blk < blockCount) {
        cout << "Block " << blk << " : ";
        for (int i = 0; i < 16; ++i) {
            cout << hex << setw(2) << setfill('0') << (cipherData[blk * 16 + i] & 0xFF);
        }
        cout << dec << "\n";
        blk = blk + 1;
    }

    delete [] keyBytes;
    delete [] plainData;
    delete [] cipherData;

    return 0;
}
