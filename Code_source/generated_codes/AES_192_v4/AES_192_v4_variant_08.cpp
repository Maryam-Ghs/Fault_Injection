#include <iostream>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

/* ---------------  AES‑192 tables (int only) --------------- */
int sbox[256] = {
    99,124,119,123,242,107,111,197, 48,  1,103, 43,254,215,171,118,
   202,130,201,125,250, 89, 71,240,173,212,162,175,156,164,114,192,
   183,  3, 12, 60, 96,  5, 98,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    // ... (full 256‑entry S‑box should be placed here) ...
    // For brevity, the full S‑box is omitted. In a real implementation,
    // fill all 256 entries according to the AES specification.
};

int rcon[10] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

/* -------------------  Helper: xtime  ------------------- */
int xtime(int x) {
    int shifted = (x << 1) & 0xFF;
    int reduced = (x & 0x80) ? 0x1B : 0;
    return shifted ^ reduced;
}

/* -------------------  AES‑192 class  ------------------- */
class AES192 {
    int *roundKeyBytes;          // (Nr+1)*16 = 208 bytes
    const int Nr = 12;           // 12 rounds for AES‑192
public:
    AES192(int *keyBytes) {
        roundKeyBytes = new int[ (Nr+1) * 16 ];
        expandKey(keyBytes);
    }
    ~AES192() {
        delete [] roundKeyBytes;
    }

    void encryptBlock(int *plain, int *cipher) {
        int *state = new int[16];
        int i = 0;
        while (i < 16) { state[i] = plain[i]; ++i; }

        addRoundKey(state, 0);

        int round = 1;
        while (round < Nr) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round * 16);
            ++round;
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, Nr * 16);

        i = 0;
        while (i < 16) { cipher[i] = state[i]; ++i; }
        delete [] state;
    }

private:
    /* ---------------  Key Expansion --------------- */
    void expandKey(int *keyBytes) {
        int *temp = new int[4];
        int wordCount = 4 * (Nr + 1);          // 52 words for AES‑192
        int *w = new int[wordCount * 4];      // each word = 4 bytes

        int i = 0;
        while (i < 24) {                       // first 6 words = 24 bytes key
            w[i] = keyBytes[i];
            ++i;
        }

        i = 6;
        while (i < wordCount) {
            int idx = (i - 1) * 4;
            temp[0] = w[idx + 0];
            temp[1] = w[idx + 1];
            temp[2] = w[idx + 2];
            temp[3] = w[idx + 3];

            if (i % 6 == 0) {
                // RotWord
                int t = temp[0];
                temp[0] = temp[1];
                temp[1] = temp[2];
                temp[2] = temp[3];
                temp[3] = t;
                // SubWord
                temp[0] = sbox[temp[0]];
                temp[1] = sbox[temp[1]];
                temp[2] = sbox[temp[2]];
                temp[3] = sbox[temp[3]];
                // Rcon
                temp[0] ^= rcon[i/6];
            } else if (i % 6 == 4) {
                // SubWord only
                temp[0] = sbox[temp[0]];
                temp[1] = sbox[temp[1]];
                temp[2] = sbox[temp[2]];
                temp[3] = sbox[temp[3]];
            }

            int prevIdx = (i - 6) * 4;
            int curIdx  = i * 4;
            w[curIdx + 0] = w[prevIdx + 0] ^ temp[0];
            w[curIdx + 1] = w[prevIdx + 1] ^ temp[1];
            w[curIdx + 2] = w[prevIdx + 2] ^ temp[2];
            w[curIdx + 3] = w[prevIdx + 3] ^ temp[3];
            ++i;
        }

        // copy to round‑key buffer
        i = 0;
        while (i < (Nr+1)*16) { roundKeyBytes[i] = w[i]; ++i; }
        delete [] w;
        delete [] temp;
    }

    /* ---------------  Core AES steps --------------- */
    void subBytes(int *st) {
        int i = 0;
        while (i < 16) { st[i] = sbox[st[i]]; ++i; }
    }

    void shiftRows(int *st) {
        int tmp[16];
        int r = 0;
        while (r < 4) {
            int c = 0;
            while (c < 4) {
                int src = r + 4 * ((c + r) % 4);
                tmp[r + 4*c] = st[src];
                ++c;
            }
            ++r;
        }
        r = 0;
        while (r < 16) { st[r] = tmp[r]; ++r; }
    }

    void mixColumns(int *st) {
        int col = 0;
        while (col < 4) {
            int i0 = col;
            int i1 = col + 4;
            int i2 = col + 8;
            int i3 = col + 12;

            int a0 = st[i0];
            int a1 = st[i1];
            int a2 = st[i2];
            int a3 = st[i3];

            int t  = a0 ^ a1 ^ a2 ^ a3;
            int u  = a0 ^ a1; u = xtime(u);
            int v  = a1 ^ a2; v = xtime(v);
            int w  = a2 ^ a3; w = xtime(w);
            int x  = a3 ^ a0; x = xtime(x);

            st[i0] ^= u ^ t;
            st[i1] ^= v ^ t;
            st[i2] ^= w ^ t;
            st[i3] ^= x ^ t;

            ++col;
        }
    }

    void addRoundKey(int *st, int offset) {
        int i = 0;
        while (i < 16) { st[i] ^= roundKeyBytes[offset + i]; ++i; }
    }
};

/* -------------------  Test driver ------------------- */
int main() {
    /* 192‑bit key (24 bytes) – sparse, clustered non‑zero values */
    int keyBytes[24] = {
        0x00,0x00,0x00,0x01,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0xFF
    };

    /* 128‑bit plaintext (16 bytes) – mostly zeros with few non‑zero bytes */
    int plainBlock[16] = {
        0x00,0x00,0x00,0x00,
        0xAA,0x00,0x00,0x00,
        0x00,0x00,0x00,0xBB,
        0x00,0x00,0x00,0x00
    };

    int cipherBlock[16];

    AES192 aes(keyBytes);
    aes.encryptBlock(plainBlock, cipherBlock);

    std::cout << "Plaintext : ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << plainBlock[i];
    std::cout << "\nKey       : ";
    for (int i = 0; i < 24; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << keyBytes[i];
    std::cout << "\nCiphertext: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << cipherBlock[i];
    std::cout << std::endl;
    return 0;
}
