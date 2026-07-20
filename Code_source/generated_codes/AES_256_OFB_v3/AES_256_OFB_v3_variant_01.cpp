#include <cstdio>
#include <cstdlib>

int main()
{
    /* LLM input variant 1: minimal-boundary */
    /* -------------------------------------------------------------
       Simple AES‑256 in OFB mode.
       All data are stored in int (0‑255).  No const, unsigned, double,
       long or external functions are used.  Everything lives inside main.
       ------------------------------------------------------------- */

    /* ---------- 1.  S‑box and R‑con (filled at runtime) ---------- */
    int sbox[256];
    int rcon[15];
    {
        int initS[256] = {
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
        for (int i = 0; i < 256; ++i) sbox[i] = initS[i];
        int initR[15] = {1,2,4,8,16,32,64,128,27,54,108,216,171,77,154};
        for (int i = 0; i < 15; ++i) rcon[i] = initR[i];
    }

    /* --------------------- 2.  Random data ---------------------- */
    srand(1);
    int keyLen = 32;                // 256‑bit key
    int *key = new int[keyLen];
    for (int i = 0; i < keyLen; ++i) key[i] = rand() & 0xFF;

    int *iv = new int[16];
    for (int i = 0; i < 16; ++i) iv[i] = rand() & 0xFF;

    int msgBytes = 16;              // minimal single block
    int *plain = new int[msgBytes];
    for (int i = 0; i < msgBytes; ++i) plain[i] = rand() & 0xFF;

    /* ------------------- 3.  Key schedule ---------------------- */
    int *roundKey = new int[240];   // 15 * 16 bytes
    // copy original key
    for (int i = 0; i < keyLen; ++i) roundKey[i] = key[i];
    // expand
    int iWord = keyLen;
    int r = 1;
    while (iWord < 240)
    {
        int temp0 = roundKey[iWord - 4];
        int temp1 = roundKey[iWord - 3];
        int temp2 = roundKey[iWord - 2];
        int temp3 = roundKey[iWord - 1];

        if (iWord % keyLen == 0)
        {
            // RotWord
            int t = temp0;
            temp0 = temp1; temp1 = temp2; temp2 = temp3; temp3 = t;
            // SubWord
            temp0 = sbox[temp0]; temp1 = sbox[temp1];
            temp2 = sbox[temp2]; temp3 = sbox[temp3];
            // Rcon
            temp0 ^= rcon[r-1];
            ++r;
        }
        else if (keyLen > 24 && (iWord % keyLen) == 16)
        {
            temp0 = sbox[temp0]; temp1 = sbox[temp1];
            temp2 = sbox[temp2]; temp3 = sbox[temp3];
        }

        roundKey[iWord + 0] = roundKey[iWord - keyLen + 0] ^ temp0;
        roundKey[iWord + 1] = roundKey[iWord - keyLen + 1] ^ temp1;
        roundKey[iWord + 2] = roundKey[iWord - keyLen + 2] ^ temp2;
        roundKey[iWord + 3] = roundKey[iWord - keyLen + 3] ^ temp3;
        iWord += 4;
    }

    /* ------------------- 4.  AES helpers ------------------------ */
    auto xtime = [&](int x)->int{
        int shifted = (x << 1) & 0xFF;
        return (x & 0x80) ? (shifted ^ 0x1B) : shifted;
    };

    auto mixColumn = [&](int *c){
        int a0 = c[0], a1 = c[1], a2 = c[2], a3 = c[3];
        int b0 = xtime(a0), b1 = xtime(a1), b2 = xtime(a2), b3 = xtime(a3);
        c[0] = b0 ^ a1 ^ b1 ^ a2 ^ a3;
        c[1] = a0 ^ b1 ^ a2 ^ b2 ^ a3;
        c[2] = a0 ^ a1 ^ b2 ^ a3 ^ b3;
        c[3] = a0 ^ b0 ^ a1 ^ a2 ^ b3;
    };

    auto subBytes = [&](int *st){
        for (int i = 0; i < 16; ++i) st[i] = sbox[st[i]];
    };

    auto shiftRows = [&](int *st){
        int tmp;

        // row 1 (shift 1)
        tmp = st[1]; st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = tmp;

        // row 2 (shift 2)
        tmp = st[2]; st[2] = st[10]; st[10] = tmp;
        tmp = st[6]; st[6] = st[14]; st[14] = tmp;

        // row 3 (shift 3)
        tmp = st[3]; st[3] = st[15]; st[15] = st[11]; st[11] = st[7]; st[7] = tmp;
    };

    auto addRoundKey = [&](int *st, int offset){
        for (int i = 0; i < 16; ++i) st[i] ^= roundKey[offset + i];
    };

    auto encryptBlock = [&](int *inp, int *out, int roundOffset){
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = inp[i];

        addRoundKey(state, roundOffset);               // round 0

        for (int r = 1; r < 14; ++r)
        {
            subBytes(state);
            shiftRows(state);
            // MixColumns
            for (int c = 0; c < 4; ++c) mixColumn(&state[4*c]);
            addRoundKey(state, roundOffset + r*16);
        }

        // final round (no MixColumns)
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, roundOffset + 14*16);

        for (int i = 0; i < 16; ++i) out[i] = state[i];
    };

    /* ------------------- 5.  OFB mode -------------------------- */
    int blocks = msgBytes / 16;
    int *cipher = new int[msgBytes];
    int feedback[16];
    for (int i = 0; i < 16; ++i) feedback[i] = iv[i];

    for (int b = 0; b < blocks; ++b)
    {
        int keystream[16];
        encryptBlock(feedback, keystream, 0);   // AES encryption of feedback

        for (int i = 0; i < 16; ++i)
            cipher[b*16 + i] = plain[b*16 + i] ^ keystream[i];

        // next feedback is the keystream (OFB)
        for (int i = 0; i < 16; ++i) feedback[i] = keystream[i];
    }

    /* ------------------- 6.  Output ---------------------------- */
    printf("Plaintext : ");
    for (int i = 0; i < msgBytes; ++i) printf("%02X", plain[i]);
    printf("\n");

    printf("Ciphertext: ");
    for (int i = 0; i < msgBytes; ++i) printf("%02X", cipher[i]);
    printf("\n");

    /* ------------------- 7.  Clean up --------------------------- */
    delete[] key;
    delete[] iv;
    delete[] plain;
    delete[] cipher;
    delete[] roundKey;

    return 0;
}
