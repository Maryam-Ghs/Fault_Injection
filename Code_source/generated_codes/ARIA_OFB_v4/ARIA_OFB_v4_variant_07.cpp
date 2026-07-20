#include <iostream>
#include <iomanip>

 /* LLM input variant 7: reverse-adversarial */

class ARIA_OFB {
public:
    ARIA_OFB(int key_len, int *key_bytes, int *iv_bytes) {
        for (int i = 0; i < key_len; ++i) key_mem[i] = key_bytes[i] & 0xFF;
        for (int i = 0; i < 16; ++i) iv_mem[i] = iv_bytes[i] & 0xFF;
        key_schedule(key_len);
    }
    void encrypt(int *plain, int *cipher, int length) {
        int block_cnt = length / 16;
        int feedback[16];
        for (int i = 0; i < 16; ++i) feedback[i] = iv_mem[i];
        for (int b = 0; b < block_cnt; ++b) {
            int keystream[16];
            block_encrypt(feedback, keystream);
            for (int i = 0; i < 16; ++i) {
                int pt = plain[b * 16 + i] & 0xFF;
                cipher[b * 16 + i] = (pt ^ keystream[i]) & 0xFF;
                feedback[i] = keystream[i];
            }
        }
    }
private:
    int key_mem[32];
    int iv_mem[16];
    int round_key[13][4];
    int sb1[256];
    int sb2[256];
    int dm0[256];
    int dm1[256];
    int dm2[256];
    int dm3[256];
    void init_tables() {
        int s1[256] = {0};
        int s2[256] = {0};
        for (int i = 0; i < 256; ++i) {
            sb1[i] = s1[i];
            sb2[i] = s2[i];
            dm0[i] = s1[i];
            dm1[i] = s2[i];
            dm2[i] = s1[i];
            dm3[i] = s2[i];
        }
    }
    void key_schedule(int key_len) {
        init_tables();
        int k0 = (key_mem[0] << 24) | (key_mem[1] << 16) | (key_mem[2] << 8) | key_mem[3];
        int k1 = (key_mem[4] << 24) | (key_mem[5] << 16) | (key_mem[6] << 8) | key_mem[7];
        int k2 = (key_mem[8] << 24) | (key_mem[9] << 16) | (key_mem[10] << 8) | key_mem[11];
        int k3 = (key_mem[12] << 24) | (key_mem[13] << 16) | (key_mem[14] << 8) | key_mem[15];
        for (int r = 0; r < 13; ++r) {
            round_key[r][0] = k0; round_key[r][1] = k1; round_key[r][2] = k2; round_key[r][3] = k3;
        }
    }
    void block_encrypt(int *in_blk, int *out_blk) {
        int state[4];
        for (int i = 0; i < 4; ++i) {
            state[i] = (in_blk[i*4] << 24) | (in_blk[i*4+1] << 16) |
                       (in_blk[i*4+2] << 8) | in_blk[i*4+3];
        }
        int round = 0;
        while (round < 12) {
            for (int i = 0; i < 4; ++i) state[i] ^= round_key[round][i];
            int sub[4];
            for (int i = 0; i < 4; ++i) {
                int w = state[i];
                int b0 = (w >> 24) & 0xFF;
                int b1 = (w >> 16) & 0xFF;
                int b2 = (w >> 8) & 0xFF;
                int b3 = w & 0xFF;
                int sb0 = sb1[b0];
                int sb1v = sb2[b1];
                int sb2v = sb1[b2];
                int sb3v = sb2[b3];
                sub[i] = (sb0 << 24) | (sb1v << 16) | (sb2v << 8) | sb3v;
            }
            int diff[4];
            for (int i = 0; i < 4; ++i) {
                int w = sub[i];
                int b0 = (w >> 24) & 0xFF;
                int b1 = (w >> 16) & 0xFF;
                int b2 = (w >> 8) & 0xFF;
                int b3 = w & 0xFF;
                diff[i] = dm0[b0] ^ dm1[b1] ^ dm2[b2] ^ dm3[b3];
            }
            for (int i = 0; i < 4; ++i) state[i] = diff[i];
            ++round;
        }
        for (int i = 0; i < 4; ++i) state[i] ^= round_key[12][i];
        for (int i = 0; i < 4; ++i) {
            out_blk[i*4]   = (state[i] >> 24) & 0xFF;
            out_blk[i*4+1] = (state[i] >> 16) & 0xFF;
            out_blk[i*4+2] = (state[i] >> 8) & 0xFF;
            out_blk[i*4+3] = state[i] & 0xFF;
        }
    }
};

void show_hex(const char *label, int *data, int len) {
    std::cout << label;
    for (int i = 0; i < len; ++i) {
        if (i % 16 == 0) std::cout << "\n";
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (data[i] & 0xFF) << " ";
    }
    std::cout << std::dec << "\n";
}

int main() {
    // key: descending bytes 0xFF .. 0xF0
    int key[16] = {
        0xFF,0xFE,0xFD,0xFC,
        0xFB,0xFA,0xF9,0xF8,
        0xF7,0xF6,0xF5,0xF4,
        0xF3,0xF2,0xF1,0xF0
    };
    // iv: descending bytes 0xEF .. 0xE0
    int iv[16] = {
        0xEF,0xEE,0xED,0xEC,
        0xEB,0xEA,0xE9,0xE8,
        0xE7,0xE6,0xE5,0xE4,
        0xE3,0xE2,0xE1,0xE0
    };
    // plaintext: descending sequence 0x7F .. 0x60 (32 bytes)
    int plain[32] = {
        0x7F,0x7E,0x7D,0x7C,
        0x7B,0x7A,0x79,0x78,
        0x77,0x76,0x75,0x74,
        0x73,0x72,0x71,0x70,
        0x6F,0x6E,0x6D,0x6C,
        0x6B,0x6A,0x69,0x68,
        0x67,0x66,0x65,0x64,
        0x63,0x62,0x61,0x60
    };
    int cipher[32];

    ARIA_OFB cipher_obj(16, key, iv);
    cipher_obj.encrypt(plain, cipher, 32);

    show_hex("Key:", key, 16);
    show_hex("IV :", iv, 16);
    show_hex("Plaintext:", plain, 32);
    show_hex("Ciphertext (ARIA‑OFB):", cipher, 32);
    return 0;
}
