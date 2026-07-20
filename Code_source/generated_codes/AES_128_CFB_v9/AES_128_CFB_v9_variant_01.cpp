/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <vector>

class AesCfb
{
public:
    AesCfb(const std::vector<int>& keyBytes, const std::vector<int>& ivBytes)
    {
        /* store IV */
        iv = ivBytes;
        /* expand the key */
        expandKey(keyBytes);
    }

    std::vector<int> encrypt(const std::vector<int>& plain)
    {
        /* CFB: encrypt IV, xor with plaintext, new IV = ciphertext */
        std::vector<int> encIv = encryptBlock(iv);
        std::vector<int> cipher(16);
        for (int i = 0; i < 16; ++i)
            cipher[i] = plain[i] ^ encIv[i];
        iv = cipher;               // update IV for next block (if any)
        return cipher;
    }

private:
    std::vector<int> roundKey;      // 176 bytes = 11 round keys
    std::vector<int> iv;            // current IV / previous ciphertext

    /* ------------------------------------------------------------------ */
    /*  Core AES operations                                                */
    /* ------------------------------------------------------------------ */
    std::vector<int> encryptBlock(const std::vector<int>& block)
    {
        std::vector<int> state = block;               // copy input
        addRoundKey(state, 0);

        for (int r = 1; r < 10; ++r)
        {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, r);
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 10);
        return state;
    }

    void subBytes(std::vector<int>& st)
    {
        for (int i = 0; i < 16; ++i)
            st[i] = sbox[st[i]];
    }

    void shiftRows(std::vector<int>& st)
    {
        int t;

        /* row 1 (rotate left 1) */
        t = st[1];  st[1] = st[5];  st[5] = st[9];  st[9] = st[13];  st[13] = t;

        /* row 2 (rotate left 2) */
        t = st[2];  st[2] = st[10]; st[10] = t;
        t = st[6];  st[6] = st[14]; st[14] = t;

        /* row 3 (rotate left 3) */
        t = st[3];  st[3] = st[15]; st[15] = st[11]; st[11] = st[7]; st[7] = t;
    }

    void mixColumns(std::vector<int>& st)
    {
        for (int c = 0; c < 4; ++c)
        {
            int i0 = c*4, i1 = i0+1, i2 = i0+2, i3 = i0+3;
            int a0 = st[i0], a1 = st[i1], a2 = st[i2], a3 = st[i3];
            int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            st[i0] = r0; st[i1] = r1; st[i2] = r2; st[i3] = r3;
        }
    }

    void addRoundKey(std::vector<int>& st, int round)
    {
        int offset = round * 16;
        for (int i = 0; i < 16; ++i)
            st[i] ^= roundKey[offset + i];
    }

    /* ------------------------------------------------------------------ */
    /*  Key schedule                                                       */
    /* ------------------------------------------------------------------ */
    void expandKey(const std::vector<int>& key)
    {
        roundKey = std::vector<int>(176);
        for (int i = 0; i < 16; ++i)
            roundKey[i] = key[i];

        int bytes = 16;
        int rconIdx = 0;

        while (bytes < 176)
        {
            int t0 = roundKey[bytes - 4];
            int t1 = roundKey[bytes - 3];
            int t2 = roundKey[bytes - 2];
            int t3 = roundKey[bytes - 1];

            if (bytes % 16 == 0)
            {
                /* RotWord */
                int tmp = t0; t0 = t1; t1 = t2; t2 = t3; t3 = tmp;
                /* SubWord */
                t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
                /* Rcon */
                t0 ^= rcon[rconIdx];
                ++rconIdx;
            }

            roundKey[bytes]     = roundKey[bytes - 16] ^ t0;
            roundKey[bytes + 1] = roundKey[bytes - 15] ^ t1;
            roundKey[bytes + 2] = roundKey[bytes - 14] ^ t2;
            roundKey[bytes + 3] = roundKey[bytes - 13] ^ t3;
            bytes += 4;
        }
    }

    /* ------------------------------------------------------------------ */
    /*  Galois field multiplication helpers                                 */
    /* ------------------------------------------------------------------ */
    int mul2(int x)
    {
        int r = x << 1;
        r ^= (r & 0x100) ? 0x11b : 0;
        return r & 0xff;
    }

    int mul3(int x)
    {
        return mul2(x) ^ x;
    }

    /* ------------------------------------------------------------------ */
    /*  Static tables                                                      */
    /* ------------------------------------------------------------------ */
    static const int sbox[256];
    static const int rcon[10];
};

/* ---------------------------------------------------------------------- */
/*  S‑Box and Rcon (plain int arrays)                                      */
/* ---------------------------------------------------------------------- */
const int AesCfb::sbox[256] = {
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

const int AesCfb::rcon[10] = {
    1,2,4,8,16,32,64,128,27,54
};

/* ---------------------------------------------------------------------- */
/*  Driver                                                               */
/* ---------------------------------------------------------------------- */
int main()
{
    /* 128‑bit key (16 bytes) – minimal boundary values (all zeros) */
    std::vector<int> key = {
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };

    /* 128‑bit IV – minimal boundary values (all zeros) */
    std::vector<int> iv = {
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };

    /* Plaintext block – minimal boundary values (all zeros) */
    std::vector<int> plain = {
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };

    AesCfb aes(key, iv);
    std::vector<int> cipher = aes.encrypt(plain);

    std::cout << "Ciphertext (hex): ";
    for (int i = 0; i < 16; ++i)
    {
        int hi = (cipher[i] >> 4) & 0xf;
        int lo = cipher[i] & 0xf;
        std::cout << (hi < 10 ? char('0'+hi) : char('a'+hi-10));
        std::cout << (lo < 10 ? char('0'+lo) : char('a'+lo-10));
        if (i != 15) std::cout << ' ';
    }
    std::cout << std::endl;
    return 0;
}
