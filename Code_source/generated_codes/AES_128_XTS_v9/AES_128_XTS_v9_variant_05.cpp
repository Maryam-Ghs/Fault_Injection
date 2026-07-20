// AES-128-XTS implementation – version #9
/* LLM input variant 5: duplicate-heavy */
// ------------------------------------------------------------
// Only int (and float) are used.  No unsigned, long, double or const.
// Manual loop unrolling, reordered arithmetic, stack arrays, helper functions.

#include <cstdio>
#include <cstring>

// -----------------------------------------------------------------
// S‑Box (filled with the standard AES values)
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
// Round constants for key schedule
int rcon[10] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36 };

// -----------------------------------------------------------------
// Helper: multiply by 2 in GF(2^8)
int xtime(int x) {
    int r = x << 1;
    r = (r & 0x100) ? (r ^ 0x11B) : r;   // 0x11B = x^8 + x^4 + x^3 + x + 1
    return r & 0xFF;
}

// Helper: multiply by 3 ( = 2 ⊕ 1 )
int mul3(int x) { return xtime(x) ^ x; }

// -----------------------------------------------------------------
// Key expansion for a 128‑bit key (produces 11 round keys)
void expandKey(int *key, int rk[11][16]) {
    // copy the original key as round 0
    for (int i = 0; i < 16; ++i) rk[0][i] = key[i];

    // generate the remaining round keys
    for (int round = 1; round <= 10; ++round) {
        // temp = last 4 bytes of previous round key
        int t0 = rk[round-1][12];
        int t1 = rk[round-1][13];
        int t2 = rk[round-1][14];
        int t3 = rk[round-1][15];

        // RotWord
        int tmp = t0; t0 = t1; t1 = t2; t2 = t3; t3 = tmp;

        // SubWord
        t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];

        // Rcon
        t0 ^= rcon[round-1];

        // first 4 bytes of this round key
        for (int i = 0; i < 4; ++i) {
            int prev = rk[round-1][i];
            int cur  = (i==0)? t0 : (i==1)? t1 : (i==2)? t2 : t3;
            rk[round][i] = prev ^ cur;
        }

        // remaining 12 bytes
        for (int i = 4; i < 16; ++i) {
            rk[round][i] = rk[round-1][i] ^ rk[round][i-4];
        }
    }
}

// -----------------------------------------------------------------
// AES core transformations – all loops are manually unrolled

void subBytes(int st[16]) {
    st[0] = sbox[st[0]];   st[1] = sbox[st[1]];   st[2] = sbox[st[2]];   st[3] = sbox[st[3]];
    st[4] = sbox[st[4]];   st[5] = sbox[st[5]];   st[6] = sbox[st[6]];   st[7] = sbox[st[7]];
    st[8] = sbox[st[8]];   st[9] = sbox[st[9]];  st[10] = sbox[st[10]]; st[11] = sbox[st[11]];
    st[12] = sbox[st[12]]; st[13] = sbox[st[13]]; st[14] = sbox[st[14]]; st[15] = sbox[st[15]];
}

void shiftRows(int st[16]) {
    // row 1 (shift left 1)
    int tmp1 = st[1];
    st[1] = st[5];
    st[5] = st[9];
    st[9] = st[13];
    st[13] = tmp1;

    // row 2 (shift left 2)
    int tmp2 = st[2];
    int tmp6 = st[6];
    st[2] = st[10];
    st[6] = st[14];
    st[10] = tmp2;
    st[14] = tmp6;

    // row 3 (shift left 3) – equivalent to right 1
    int tmp3 = st[3];
    st[3] = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7] = tmp3;
}

void mixColumns(int st[16]) {
    for (int c = 0; c < 4; ++c) {
        int i = c*4;
        int a0 = st[i];   int a1 = st[i+1];
        int a2 = st[i+2]; int a3 = st[i+3];

        int r0 = xtime(a0) ^ mul3(a1) ^ a2 ^ a3;
        int r1 = a0 ^ xtime(a1) ^ mul3(a2) ^ a3;
        int r2 = a0 ^ a1 ^ xtime(a2) ^ mul3(a3);
        int r3 = mul3(a0) ^ a1 ^ a2 ^ xtime(a3);

        st[i]   = r0 & 0xFF;
        st[i+1] = r1 & 0xFF;
        st[i+2] = r2 & 0xFF;
        st[i+3] = r3 & 0xFF;
    }
}

void addRoundKey(int st[16], int *rk) {
    // XOR each byte with the round key byte
    st[0] ^= rk[0];  st[1] ^= rk[1];  st[2] ^= rk[2];  st[3] ^= rk[3];
    st[4] ^= rk[4];  st[5] ^= rk[5];  st[6] ^= rk[6];  st[7] ^= rk[7];
    st[8] ^= rk[8];  st[9] ^= rk[9];  st[10] ^= rk[10]; st[11] ^= rk[11];
    st[12] ^= rk[12];st[13] ^= rk[13];st[14] ^= rk[14];st[15] ^= rk[15];
}

// -----------------------------------------------------------------
// Full AES‑128 encryption (10 rounds) – manual unrolling

void aesEncryptBlock(int in[16], int out[16], int rk[11][16]) {
    int state[16];
    for (int i = 0; i < 16; ++i) state[i] = in[i];

    // round 0
    addRoundKey(state, rk[0]);

    // round 1
    subBytes(state);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, rk[1]);

    // round 2
    subBytes(state);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, rk[2]);

    // round 3
    subBytes(state);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, rk[3]);

    // round 4
    subBytes(state);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, rk[4]);

    // round 5
    subBytes(state);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, rk[5]);

    // round 6
    subBytes(state);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, rk[6]);

    // round 7
    subBytes(state);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, rk[7]);

    // round 8
    subBytes(state);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, rk[8]);

    // round 9
    subBytes(state);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, rk[9]);

    // round 10 (final, no MixColumns)
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, rk[10]);

    // copy to output
    for (int i = 0; i < 16; ++i) out[i] = state[i];
}

// -----------------------------------------------------------------
// GF(2^128) multiplication by α (the primitive element)
// Input/output is a 16‑byte little‑endian array

void mulAlpha(int t[16]) {
    int carry = 0;
    for (int i = 0; i < 16; ++i) {
        int nxt = (t[i] & 0x80) ? 1 : 0;
        t[i] = ((t[i] << 1) & 0xFF) ^ carry;
        carry = nxt;
    }
    // if there was an overflow from the most‑significant byte,
    // XOR with the reduction polynomial (0x87)
    if (carry) t[0] ^= 0x87;
}

// -----------------------------------------------------------------
// XTS encryption for a single 16‑byte block

void xtsEncryptOneBlock(int pt[16], int ct[16],
                        int dataKey[11][16], int tweakKey[11][16],
                        int iv[16]) {
    int tweak[16];
    aesEncryptBlock(iv, tweak, tweakKey);          // encrypt the tweak (sector number)

    int xored[16];
    // X = plaintext XOR tweak
    for (int i = 0; i < 16; ++i) xored[i] = pt[i] ^ tweak[i];

    int encrypted[16];
    aesEncryptBlock(xored, encrypted, dataKey);    // AES(dataKey, X)

    // ciphertext = encrypted XOR tweak
    for (int i = 0; i < 16; ++i) ct[i] = encrypted[i] ^ tweak[i];
}

// -----------------------------------------------------------------
// Utility: print a block as hex

void printHex(const char *label, int blk[16]) {
    printf("%s", label);
    for (int i = 0; i < 16; ++i) {
        printf("%02X", blk[i] & 0xFF);
    }
    printf("\n");
}

// -----------------------------------------------------------------
// Main – deterministic test vectors, no external input

int main() {
    // ----- deterministic keys (16 bytes each) -----
    // Duplicate-heavy pattern: all bytes are 0x55
    int keyData[16]  = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
                        0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};

    // Duplicate-heavy tweak key: also all 0x55
    int keyTweak[16] = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
                        0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};

    // ----- deterministic IV (tweak) – sector number 0 (all zeros)
    int iv[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    // ----- plaintext block -----
    // Duplicate-heavy plaintext: all bytes are 0xFF
    int plain[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    // ----- expand both keys -----
    int roundKeyData[11][16];
    int roundKeyTweak[11][16];
    expandKey(keyData,  roundKeyData);
    expandKey(keyTweak, roundKeyTweak);

    // ----- perform XTS encryption on the single block -----
    int cipher[16];
    xtsEncryptOneBlock(plain, cipher,
                       roundKeyData, roundKeyTweak,
                       iv);

    // ----- output -----
    printHex("Plaintext : ", plain);
    printHex("Ciphertext: ", cipher);

    return 0;
}
