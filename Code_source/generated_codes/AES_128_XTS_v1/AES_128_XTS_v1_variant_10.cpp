#include <iostream>
#include <iomanip>
#include <cstring>

/* LLM input variant 10: large-safe-stress */

class aes_xts_128 {
public:
    aes_xts_128(int *k_data, int *k_tweak) {
        drk = new int[176];
        trk = new int[176];
        key_expansion(k_data, drk);
        key_expansion(k_tweak, trk);
    }

    ~aes_xts_128() {
        delete[] drk;
        delete[] trk;
    }

    void encrypt_xts(int *plain, int *iv, int *out) {
        int tweak[16];
        encrypt_block(iv, trk, tweak);

        int xored[16];
        for (int i = 0; i < 16; ++i)
            xored[i] = plain[i] ^ tweak[i];

        int enc[16];
        encrypt_block(xored, drk, enc);

        for (int i = 0; i < 16; ++i)
            out[i] = enc[i] ^ tweak[i];
    }

private:
    int *drk;
    int *trk;

    static void init_sbox(int *sb) {
        int temp[256] = {
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
        for (int i = 0; i < 256; ++i) sb[i] = temp[i];
    }

    static void init_rcon(int *rc) {
        int tmp[11] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C};
        for (int i = 0; i < 11; ++i) rc[i] = tmp[i];
    }

    void key_expansion(int *key, int *out) {
        int sbox[256];
        int rcon[11];
        init_sbox(sbox);
        init_rcon(rcon);
        for (int i = 0; i < 16; ++i) out[i] = key[i];
        int bytes = 16;
        int i = 1;
        while (bytes < 176) {
            int t0 = out[bytes-4];
            int t1 = out[bytes-3];
            int t2 = out[bytes-2];
            int t3 = out[bytes-1];
            int rot = t1;
            t1 = t2; t2 = t3; t3 = t0; t0 = rot;
            t0 = sbox[t0];
            t1 = sbox[t1];
            t2 = sbox[t2];
            t3 = sbox[t3];
            t0 ^= rcon[i-1];
            out[bytes]     = out[bytes-16] ^ t0;
            out[bytes+1]   = out[bytes-15] ^ t1;
            out[bytes+2]   = out[bytes-14] ^ t2;
            out[bytes+3]   = out[bytes-13] ^ t3;
            for (int j = 4; j < 16; ++j) {
                out[bytes + j] = out[bytes + j - 4] ^ out[bytes + j - 16];
            }
            bytes += 16;
            ++i;
        }
    }

    static int mul2(int x) {
        int r = x << 1;
        if (r & 0x100) r ^= 0x11B;
        return r & 0xFF;
    }

    static int mul3(int x) {
        return mul2(x) ^ x;
    }

    void encrypt_block(int *in, int *rk, int *out) {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = in[i] ^ rk[i];
        int sbox[256];
        init_sbox(sbox);
        for (int round = 1; round <= 9; ++round) {
            for (int b = 0; b < 16; ++b) state[b] = sbox[state[b]];
            int tmp0 = state[1];
            int tmp1 = state[5];
            int tmp2 = state[9];
            int tmp3 = state[13];
            state[1] = tmp0; state[5] = tmp1; state[9] = tmp2; state[13] = tmp3;
            tmp0 = state[2]; tmp1 = state[6]; tmp2 = state[10]; tmp3 = state[14];
            state[2] = tmp0; state[6] = tmp1; state[10] = tmp2; state[14] = tmp3;
            tmp0 = state[3]; tmp1 = state[7]; tmp2 = state[11]; tmp3 = state[15];
            state[3] = tmp0; state[7] = tmp1; state[11] = tmp2; state[15] = tmp3;
            for (int col = 0; col < 4; ++col) {
                int i0 = state[col*4];
                int i1 = state[col*4+1];
                int i2 = state[col*4+2];
                int i3 = state[col*4+3];
                int m0 = mul2(i0) ^ mul3(i1) ^ i2 ^ i3;
                int m1 = i0 ^ mul2(i1) ^ mul3(i2) ^ i3;
                int m2 = i0 ^ i1 ^ mul2(i2) ^ mul3(i3);
                int m3 = mul3(i0) ^ i1 ^ i2 ^ mul2(i3);
                state[col*4]   = m0;
                state[col*4+1] = m1;
                state[col*4+2] = m2;
                state[col*4+3] = m3;
            }
            int rk_off = round*16;
            for (int b = 0; b < 16; ++b) state[b] ^= rk[rk_off + b];
        }
        for (int b = 0; b < 16; ++b) state[b] = sbox[state[b]];
        int t0 = state[1];
        int t1 = state[5];
        int t2 = state[9];
        int t3 = state[13];
        state[1] = t0; state[5] = t1; state[9] = t2; state[13] = t3;
        t0 = state[2]; t1 = state[6]; t2 = state[10]; t3 = state[14];
        state[2] = t0; state[6] = t1; state[10] = t2; state[14] = t3;
        t0 = state[3]; t1 = state[7]; t2 = state[11]; t3 = state[15];
        state[3] = t0; state[7] = t1; state[11] = t2; state[15] = t3;
        int rk_off = 160;
        for (int b = 0; b < 16; ++b) out[b] = state[b] ^ rk[rk_off + b];
    }
};

int main() {
    // Data key (16 bytes)
    int data_key[16] = {
        0x27,0x18,0x28,0x23,0x55,0x2c,0x6e,0xF6,
        0x57,0x23,0x39,0x86,0x6c,0xB2,0x19,0x0b
    };
    // Tweak key (16 bytes)
    int tweak_key[16] = {
        0x5c,0x2b,0x4c,0x0c,0x0e,0x5c,0x76,0x84,
        0x7c,0x4b,0x5e,0x0f,0x31,0x6e,0x3c,0x2c
    };
    // Base IV (tweak) – 16 bytes
    int base_iv[16] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
    };

    const int BLOCKS = 64;               // 64 blocks = 1024 bytes, safe size
    const int TOTAL_ELEMS = BLOCKS * 16;

    // Prepare deterministic plaintext: incremental pattern 0x00..0xFF repeating
    int *plaintext = new int[TOTAL_ELEMS];
    for (int i = 0; i < TOTAL_ELEMS; ++i) {
        plaintext[i] = i & 0xFF;
    }

    // Allocate output buffer
    int *ciphertext = new int[TOTAL_ELEMS];

    aes_xts_128 engine(data_key, tweak_key);

    int iv[16];
    for (int blk = 0; blk < BLOCKS; ++blk) {
        std::memcpy(iv, base_iv, sizeof(iv));
        // Simple tweak variation: add block index to last byte (mod 256)
        iv[15] = (iv[15] + blk) & 0xFF;
        engine.encrypt_xts(plaintext + blk*16, iv, ciphertext + blk*16);
    }

    // Print all ciphertext blocks
    std::cout << "Ciphertext (XTS, " << BLOCKS << " blocks):\n";
    for (int blk = 0; blk < BLOCKS; ++blk) {
        std::cout << "Block " << std::dec << blk << ": ";
        for (int i = 0; i < 16; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << (ciphertext[blk*16 + i] & 0xFF);
        }
        std::cout << std::dec << "\n";
    }

    delete[] plaintext;
    delete[] ciphertext;
    return 0;
}
