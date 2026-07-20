#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

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

int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

void SubBytes(int *st)
{
    int idx = 0;
    while (idx < 16) {
        st[idx] = sbox[st[idx]];
        idx = idx + 1;
    }
}

void ShiftRows(int *st)
{
    int tmp;

    /* Row 1 shift by 1 */
    tmp = st[1];
    st[1] = st[5];
    st[5] = st[9];
    st[9] = st[13];
    st[13] = tmp;

    /* Row 2 shift by 2 */
    tmp = st[2];
    st[2] = st[10];
    st[10] = tmp;
    tmp = st[6];
    st[6] = st[14];
    st[14] = tmp;

    /* Row 3 shift by 3 (right by 1) */
    tmp = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7] = st[3];
    st[3] = tmp;
}

int xtime(int x)
{
    int hi = (x & 0x80);
    int res = (x << 1) & 0xFF;
    if (hi != 0) res = res ^ 0x1B;
    return res;
}

void MixColumns(int *st)
{
    int i = 0;
    while (i < 4) {
        int a0 = st[4*i];
        int a1 = st[4*i+1];
        int a2 = st[4*i+2];
        int a3 = st[4*i+3];

        int t0 = xtime(a0) ^ xtime(a1) ^ a1 ^ a2 ^ a3;
        int t1 = a0 ^ xtime(a1) ^ xtime(a2) ^ a2 ^ a3;
        int t2 = a0 ^ a1 ^ xtime(a2) ^ xtime(a3) ^ a3;
        int t3 = xtime(a0) ^ a0 ^ a1 ^ a2 ^ xtime(a3);

        st[4*i]   = t0;
        st[4*i+1] = t1;
        st[4*i+2] = t2;
        st[4*i+3] = t3;

        i = i + 1;
    }
}

void AddRoundKey(int *st, int *rk)
{
    int i = 0;
    while (i < 16) {
        st[i] = st[i] ^ rk[i];
        i = i + 1;
    }
}

void KeyExpansion(int *key, int *expk)
{
    int i = 0;
    while (i < 16) {
        expk[i] = key[i];
        i = i + 1;
    }

    int bytes = 16;
    int rconIdx = 1;
    while (bytes < 176) {
        int temp0 = expk[bytes-4];
        int temp1 = expk[bytes-3];
        int temp2 = expk[bytes-2];
        int temp3 = expk[bytes-1];

        if ((bytes % 16) == 0) {
            int t = temp0;
            temp0 = sbox[temp1] ^ rcon[rconIdx];
            temp1 = sbox[temp2];
            temp2 = sbox[temp3];
            temp3 = sbox[t];
            rconIdx = rconIdx + 1;
        }

        expk[bytes]   = expk[bytes-16] ^ temp0;
        expk[bytes+1] = expk[bytes-15] ^ temp1;
        expk[bytes+2] = expk[bytes-14] ^ temp2;
        expk[bytes+3] = expk[bytes-13] ^ temp3;
        bytes = bytes + 4;
    }
}

void CipherBlock(int *in, int *out, int *expk)
{
    int st[16];
    int i = 0;
    while (i < 16) {
        st[i] = in[i];
        i = i + 1;
    }

    AddRoundKey(st, expk);

    int round = 1;
    while (round < 10) {
        SubBytes(st);
        ShiftRows(st);
        MixColumns(st);
        AddRoundKey(st, expk + 16*round);
        round = round + 1;
    }

    SubBytes(st);
    ShiftRows(st);
    AddRoundKey(st, expk + 160);

    i = 0;
    while (i < 16) {
        out[i] = st[i];
        i = i + 1;
    }
}

void OFBEncrypt(int *plain, int *cipher, int len, int *key, int *iv)
{
    int expanded[176];
    KeyExpansion(key, expanded);

    int feedback[16];
    int i = 0;
    while (i < 16) {
        feedback[i] = iv[i];
        i = i + 1;
    }

    int blockIdx = 0;
    while (blockIdx < len) {
        int ks[16];
        CipherBlock(feedback, ks, expanded);

        int j = 0;
        while (j < 16 && (blockIdx + j) < len) {
            cipher[blockIdx + j] = plain[blockIdx + j] ^ ks[j];
            j = j + 1;
        }

        i = 0;
        while (i < 16) {
            feedback[i] = ks[i];
            i = i + 1;
        }

        blockIdx = blockIdx + 16;
    }
}

int main()
{
    /* deterministic seed for reproducible large test */
    std::srand(0);

    const int dataSize = 1 << 20; // 1,048,576 bytes (1 MiB)
    int *plain   = new int[dataSize];
    int *cipher  = new int[dataSize];
    int *key     = new int[16];
    int *iv      = new int[16];

    for (int i = 0; i < dataSize; ++i) {
        plain[i] = (i * 37) % 256;
    }

    for (int i = 0; i < 16; ++i) {
        key[i] = (i * 13 + 7) % 256;
        iv[i]  = (i * 23 + 11) % 256;
    }

    OFBEncrypt(plain, cipher, dataSize, key, iv);

    std::cout << "Ciphertext (hex):\n";
    for (int i = 0; i < dataSize; ++i) {
        int hi = cipher[i] / 16;
        int lo = cipher[i] % 16;
        char h = (hi < 10) ? ('0' + hi) : ('A' + hi - 10);
        char l = (lo < 10) ? ('0' + lo) : ('A' + lo - 10);
        std::cout << h << l << ' ';
        if ((i + 1) % 16 == 0) std::cout << '\n';
    }

    delete[] plain;
    delete[] cipher;
    delete[] key;
    delete[] iv;
    return 0;
}
