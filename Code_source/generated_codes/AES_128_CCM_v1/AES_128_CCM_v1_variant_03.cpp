/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <iomanip>

class AesCcm {
    int sbox[256];
    int rcon[10];
    int roundKey[176];                 // 11 * 16 bytes = 176
    int block[16];
    int temp[4];

    // ------- Helper arithmetic (branch‑free) -------
    int mul2(int x) {
        int m = x << 1;
        m ^= 0x1b & -(x >> 7);
        return m & 0xff;
    }
    int mul3(int x) {
        return (mul2(x) ^ x) & 0xff;
    }

    // ------- Core AES steps -------
    void subBytes() {
        for (int i = 0; i < 16; ++i) {
            block[i] = sbox[block[i]];
        }
    }
    void shiftRows() {
        // row 1
        int t1 = block[1];
        block[1] = block[5];
        block[5] = block[9];
        block[9] = block[13];
        block[13] = t1;
        // row 2
        int t2 = block[2];
        int t6 = block[6];
        block[2] = block[10];
        block[6] = block[14];
        block[10] = t2;
        block[14] = t6;
        // row 3
        int t3 = block[3];
        block[3] = block[15];
        block[15] = block[11];
        block[11] = block[7];
        block[7] = t3;
    }
    void mixColumns() {
        for (int c = 0; c < 4; ++c) {
            int i0 = c * 4;
            int a0 = block[i0];
            int a1 = block[i0 + 1];
            int a2 = block[i0 + 2];
            int a3 = block[i0 + 3];
            int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            block[i0]     = r0 & 0xff;
            block[i0 + 1] = r1 & 0xff;
            block[i0 + 2] = r2 & 0xff;
            block[i0 + 3] = r3 & 0xff;
        }
    }
    void addRoundKey(int round) {
        int offset = round * 16;
        for (int i = 0; i < 16; ++i) {
            block[i] ^= roundKey[offset + i];
        }
    }

    // ------- Key schedule -------
    void keyExpansion(const int *key) {
        for (int i = 0; i < 16; ++i) {
            roundKey[i] = key[i];
        }
        int bytes = 16;
        int r = 0;
        while (bytes < 176) {
            for (int i = 0; i < 4; ++i) {
                temp[i] = roundKey[bytes - 4 + i];
            }
            if (bytes % 16 == 0) {
                // RotWord
                int t = temp[0];
                temp[0] = temp[1];
                temp[1] = temp[2];
                temp[2] = temp[3];
                temp[3] = t;
                // SubWord
                for (int i = 0; i < 4; ++i) {
                    temp[i] = sbox[temp[i]];
                }
                // Rcon
                temp[0] ^= rcon[r];
                ++r;
            }
            for (int i = 0; i < 4; ++i) {
                roundKey[bytes] = roundKey[bytes - 16] ^ temp[i];
                ++bytes;
            }
        }
    }

    // ------- Single block encryption -------
    void encryptBlock(const int *in, int *out) {
        for (int i = 0; i < 16; ++i) block[i] = in[i];
        addRoundKey(0);
        for (int round = 1; round < 10; ++round) {
            subBytes();
            shiftRows();
            mixColumns();
            addRoundKey(round);
        }
        subBytes();
        shiftRows();
        addRoundKey(10);
        for (int i = 0; i < 16; ++i) out[i] = block[i];
    }

    // ------- CBC‑MAC (single block for demo) -------
    void cbcMac(const int *msg, int mlen, const int *nonce, int nlen,
                const int *adata, int alen, int *mac) {
        int x[16] = {0};
        // B0
        int flags = ( (alen > 0) ? 0x40 : 0 ) | ((nlen - 1) << 3) | (15 - 16 + 1);
        x[0] = flags;
        for (int i = 0; i < nlen; ++i) x[1 + i] = nonce[i];
        for (int i = 0; i < 15 - nlen - 1; ++i) x[1 + nlen + i] = 0;
        // length encoding (mlen fits in 2 bytes for demo)
        x[15] = mlen & 0xff;
        x[14] = (mlen >> 8) & 0xff;
        encryptBlock(x, x);
        // AAD (skip if alen==0)
        for (int i = 0; i < alen; ++i) {
            x[i % 16] ^= adata[i];
            if ((i % 16) == 15) encryptBlock(x, x);
        }
        if (alen % 16) encryptBlock(x, x);
        // Message
        for (int i = 0; i < mlen; ++i) {
            x[i % 16] ^= msg[i];
            if ((i % 16) == 15) encryptBlock(x, x);
        }
        if (mlen % 16) encryptBlock(x, x);
        for (int i = 0; i < 16; ++i) mac[i] = x[i];
    }

    // ------- CTR encryption -------
    void ctrEncrypt(const int *msg, int mlen, const int *nonce, int nlen,
                    const int *mac, int tlen, int *out) {
        int counter[16] = {0};
        // Flags for CTR
        counter[0] = nlen - 1;
        for (int i = 0; i < nlen; ++i) counter[1 + i] = nonce[i];
        // Counter starts at 1
        int ctr = 1;
        for (int i = 0; i < mlen; ++i) {
            if ((i % 16) == 0) {
                // build counter block
                counter[15] = ctr & 0xff;
                counter[14] = (ctr >> 8) & 0xff;
                counter[13] = (ctr >> 16) & 0xff;
                counter[12] = (ctr >> 24) & 0xff;
                encryptBlock(counter, counter);
                ++ctr;
            }
            out[i] = msg[i] ^ counter[i % 16];
        }
        // Append MAC encrypted with first counter block (ctr == 0)
        // Build counter block with ctr = 0
        for (int i = 0; i < 16; ++i) counter[i] = 0;
        counter[0] = nlen - 1;
        for (int i = 0; i < nlen; ++i) counter[1 + i] = nonce[i];
        encryptBlock(counter, counter);
        for (int i = 0; i < tlen; ++i) {
            out[mlen + i] = mac[i] ^ counter[i];
        }
    }

public:
    AesCcm() {
        // Initialise S‑box (first 256 entries)
        int s[256] = {
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
        for (int i = 0; i < 256; ++i) sbox[i] = s[i];
        // Rcon
        int rc[10] = {1,2,4,8,16,32,64,128,27,54};
        for (int i = 0; i < 10; ++i) rcon[i] = rc[i];
    }

    void encrypt(const int *plain, int plen,
                 const int *nonce, int nlen,
                 const int *adata, int alen,
                 int *cipher, int &clen) {
        int mac[16];
        cbcMac(plain, plen, nonce, nlen, adata, alen, mac);
        int tlen = 8;                     // 64‑bit tag for demo
        ctrEncrypt(plain, plen, nonce, nlen, mac, tlen, cipher);
        clen = plen + tlen;
    }
};

int main() {
    // ----- Demo data (all on stack) -----
    int key[16] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    int nonce[12] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    int aad[0] = { };   // empty AAD
    int plain[32] = {
        // 32‑byte message (alternating zeros and ones)
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01
    };
    int cipher[40];          // enough for ciphertext + tag
    int outlen = 0;

    // ----- Initialise AES‑CCM -----
    AesCcm engine;
    // key schedule (internal to object)
    engine.encrypt(plain, 32, nonce, 12, aad, 0, cipher, outlen);

    // ----- Print result -----
    std::cout << "Ciphertext + Tag (" << outlen << " bytes):\n";
    for (int i = 0; i < outlen; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xff);
        if ((i + 1) % 16 == 0) std::cout << "\n";
        else std::cout << " ";
    }
    return 0;
}
