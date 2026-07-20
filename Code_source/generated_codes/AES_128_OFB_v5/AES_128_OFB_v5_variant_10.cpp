#include <iostream>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

//------------------------------------------------------------
//  AES-128 implementation (only int and float types are used)
//------------------------------------------------------------

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

int rcon[11] = {0x00000000,0x01000000,0x02000000,0x04000000,0x08000000,
                0x10000000,0x20000000,0x40000000,0x80000000,0x1b000000,
                0x36000000};

//------------------------------------------------------------
//  Helper functions (all use int)
//------------------------------------------------------------
int xtime(int x) {
    int m = x & 0x80;
    x = (x << 1) & 0xff;
    if (m) x ^= 0x1b;
    return x;
}

void subWord(int *w) {
    int i = 0;
    while (i < 4) {
        w[i] = sbox[w[i]];
        i = i + 1;
    }
}

void rotWord(int *w) {
    int tmp = w[0];
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = tmp;
}

//------------------------------------------------------------
//  AES class (stack based, no const, no unsigned)
//------------------------------------------------------------
class aes128_ofb {
public:
    void setKey(const int *keyBytes) {
        int i = 0;
        while (i < 4) {
            roundKey[i][0] = keyBytes[4*i];
            roundKey[i][1] = keyBytes[4*i+1];
            roundKey[i][2] = keyBytes[4*i+2];
            roundKey[i][3] = keyBytes[4*i+3];
            i = i + 1;
        }

        i = 4;
        while (i < 44) {
            int temp[4];
            int j = 0;
            while (j < 4) {
                temp[j] = roundKey[i-1][j];
                j = j + 1;
            }
            if (i % 4 == 0) {
                rotWord(temp);
                subWord(temp);
                temp[0] = temp[0] ^ ((rcon[i/4] >> 24) & 0xff);
            }
            j = 0;
            while (j < 4) {
                roundKey[i][j] = roundKey[i-4][j] ^ temp[j];
                j = j + 1;
            }
            i = i + 1;
        }
    }

    void encryptBlock(const int *in, int *out) {
        int state[4][4];
        int i = 0, j = 0;
        while (i < 4) {
            j = 0;
            while (j < 4) {
                state[j][i] = in[4*i + j];
                j = j + 1;
            }
            i = i + 1;
        }

        addRoundKey(state, 0);

        i = 1;
        while (i <= 9) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, i);
            i = i + 1;
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 10);

        i = 0;
        while (i < 4) {
            j = 0;
            while (j < 4) {
                out[4*i + j] = state[j][i];
                j = j + 1;
            }
            i = i + 1;
        }
    }

    void ofbEncrypt(const int *plain, int plainLen,
                    const int *iv,
                    int *cipher) {
        int curIv[16];
        int i = 0;
        while (i < 16) {
            curIv[i] = iv[i];
            i = i + 1;
        }

        int offset = 0;
        while (offset < plainLen) {
            int keystream[16];
            encryptBlock(curIv, keystream);
            i = 0;
            while (i < 16 && offset + i < plainLen) {
                cipher[offset + i] = plain[offset + i] ^ keystream[i];
                i = i + 1;
            }
            i = 0;
            while (i < 16) {
                curIv[i] = keystream[i];
                i = i + 1;
            }
            offset = offset + 16;
        }
    }

private:
    int roundKey[44][4];

    void addRoundKey(int st[4][4], int round) {
        int i = 0;
        while (i < 4) {
            int j = 0;
            while (j < 4) {
                st[j][i] = st[j][i] ^ roundKey[round*4 + i][j];
                j = j + 1;
            }
            i = i + 1;
        }
    }

    void subBytes(int st[4][4]) {
        int i = 0;
        while (i < 4) {
            int j = 0;
            while (j < 4) {
                st[i][j] = sbox[st[i][j]];
                j = j + 1;
            }
            i = i + 1;
        }
    }

    void shiftRows(int st[4][4]) {
        // row 1 shift left by 1
        int tmp = st[1][0];
        st[1][0] = st[1][1];
        st[1][1] = st[1][2];
        st[1][2] = st[1][3];
        st[1][3] = tmp;

        // row 2 shift left by 2
        tmp = st[2][0];
        st[2][0] = st[2][2];
        st[2][2] = tmp;
        tmp = st[2][1];
        st[2][1] = st[2][3];
        st[2][3] = tmp;

        // row 3 shift left by 3 (right by 1)
        tmp = st[3][3];
        st[3][3] = st[3][2];
        st[3][2] = st[3][1];
        st[3][1] = st[3][0];
        st[3][0] = tmp;
    }

    void mixColumns(int st[4][4]) {
        int i = 0;
        while (i < 4) {
            int a0 = st[0][i];
            int a1 = st[1][i];
            int a2 = st[2][i];
            int a3 = st[3][i];
            int r0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            int r1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            int r2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            int r3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
            st[0][i] = r0 & 0xff;
            st[1][i] = r1 & 0xff;
            st[2][i] = r2 & 0xff;
            st[3][i] = r3 & 0xff;
            i = i + 1;
        }
    }
};

//------------------------------------------------------------
//  Main – deterministic large test vector
//------------------------------------------------------------
int main() {
    // key  = 2b7e1516 28aed2a6 abf71588 09cf4f3c
    int key[16] = {
        0x2b,0x7e,0x15,0x16,
        0x28,0xae,0xd2,0xa6,
        0xab,0xf7,0x15,0x88,
        0x09,0xcf,0x4f,0x3c
    };

    // Generate a larger plaintext of 256 bytes (16 blocks) with a simple pattern
    const int plainLen = 256;
    int plain[plainLen];
    int i = 0;
    while (i < plainLen) {
        plain[i] = i & 0xff; // byte values 0x00..0xff repeating
        i = i + 1;
    }

    // IV = 00010203 04050607 08090a0b 0c0d0e0f
    int iv[16] = {
        0x00,0x01,0x02,0x03,
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,
        0x0c,0x0d,0x0e,0x0f
    };

    int cipher[plainLen];
    aes128_ofb engine;
    engine.setKey(key);
    engine.ofbEncrypt(plain, plainLen, iv, cipher);

    std::cout << "Ciphertext (hex): ";
    i = 0;
    while (i < plainLen) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << cipher[i];
        i = i + 1;
    }
    std::cout << std::dec << std::endl;
    return 0;
}
