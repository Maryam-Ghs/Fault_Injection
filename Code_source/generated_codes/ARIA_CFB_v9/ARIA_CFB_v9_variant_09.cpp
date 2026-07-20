/* LLM input variant 9: medium-deterministic-random */
// ARIA-CFB implementation – version #9 (medium deterministic random variant)
#include <iostream>
#include <iomanip>
#include <cstdlib>

int main() {
    // --------------------------------------------------------
    // 1. Parameters and deterministic pseudo‑random data (LCG)
    // --------------------------------------------------------
    int blockSize = 16;
    int keyLen    = 16;
    int dataLen   = 64;

    int *key   = new int[keyLen];
    int *iv   = new int[blockSize];
    int *plain = new int[dataLen];
    int *cipher= new int[dataLen];
    int *dec   = new int[dataLen];

    // Simple linear congruential generator: X_{n+1} = (a*X_n + c) mod 256
    unsigned int seed = 12345;
    auto lcg = [&](unsigned int &x) {
        x = (1103515245u * x + 12345u) & 0xFFu;
        return (int)x;
    };

    for (int i = 0; i < keyLen; ++i)   key[i] = lcg(seed);
    for (int i = 0; i < blockSize; ++i) iv[i] = lcg(seed);
    for (int i = 0; i < dataLen; ++i) plain[i] = lcg(seed);

    // --------------------------------------------------------
    // 2. ARIA building blocks (S‑box, GF(2^8) multiplication,
    //    diffusion ...)

    int S1[256] = {
        0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
        0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
        0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
        0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
        0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
        0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
        0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
        0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
        0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
        0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
        0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
        0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
        0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
        0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
        0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
        0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
    };

    // --------------------------------------------------------
    // 3. Helper arithmetic in GF(2^8)
    // --------------------------------------------------------
    auto xtime = [](int a)->int {
        int r = a << 1;
        if (r & 0x100) r ^= 0x11b;
        return r & 0xff;
    };
    auto mul2 = [&](int a)->int { return xtime(a); };
    auto mul3 = [&](int a)->int { return xtime(a) ^ a; };

    // --------------------------------------------------------
    // 4. Diffusion layer (M‑DS)
    // --------------------------------------------------------
    auto diffuse = [&](int *st) {
        int tmp[16];
        for (int col = 0; col < 4; ++col) {
            int a0 = st[col*4+0];
            int a1 = st[col*4+1];
            int a2 = st[col*4+2];
            int a3 = st[col*4+3];
            tmp[col*4+0] = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            tmp[col*4+1] = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            tmp[col*4+2] = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            tmp[col*4+3] = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
        }
        for (int i = 0; i < 16; ++i) st[i] = tmp[i];
    };

    // --------------------------------------------------------
    // 5. Substitution
    // --------------------------------------------------------
    auto subBytes = [&](int *st, int *box) {
        for (int i = 0; i < 16; ++i) st[i] = box[st[i]];
    };

    // --------------------------------------------------------
    // 6. Simplified ARIA block encryption
    // --------------------------------------------------------
    auto encryptBlock = [&](int *in, int *out, int *rk) {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = in[i] ^ rk[i];
        for (int r = 1; r <= 11; ++r) {
            if (r & 1) subBytes(state, S1);
            else       subBytes(state, S1);
            diffuse(state);
            for (int i = 0; i < 16; ++i) state[i] ^= rk[i];
        }
        subBytes(state, S1);
        for (int i = 0; i < 16; ++i) out[i] = state[i] ^ rk[i];
    };

    // --------------------------------------------------------
    // 7. CFB mode (encryption)
    // --------------------------------------------------------
    int *feedback = new int[blockSize];
    for (int i = 0; i < blockSize; ++i) feedback[i] = iv[i];
    int tmp[16];

    for (int blk = 0; blk < dataLen; blk += blockSize) {
        encryptBlock(feedback, tmp, key);
        for (int j = 0; j < blockSize; ++j) {
            cipher[blk + j] = plain[blk + j] ^ tmp[j];
            feedback[j] = cipher[blk + j];
        }
    }

    // --------------------------------------------------------
    // 8. CFB mode (decryption)
    // --------------------------------------------------------
    for (int i = 0; i < blockSize; ++i) feedback[i] = iv[i];
    for (int blk = 0; blk < dataLen; blk += blockSize) {
        encryptBlock(feedback, tmp, key);
        for (int j = 0; j < blockSize; ++j) {
            dec[blk + j] = cipher[blk + j] ^ tmp[j];
            feedback[j] = cipher[blk + j];
        }
    }

    // --------------------------------------------------------
    // 9. Output (hexadecimal)
    // --------------------------------------------------------
    std::cout << "Key      : ";
    for (int i = 0; i < keyLen; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << key[i];
    std::cout << "\nIV       : ";
    for (int i = 0; i < blockSize; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << iv[i];
    std::cout << "\nPlaintext: ";
    for (int i = 0; i < dataLen; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i];
    std::cout << "\nCiphertext: ";
    for (int i = 0; i < dataLen; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << cipher[i];
    std::cout << "\nDecrypted : ";
    for (int i = 0; i < dataLen; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << dec[i];
    std::cout << std::dec << "\n";

    // --------------------------------------------------------
    // 10. Clean‑up
    // --------------------------------------------------------
    delete[] key;
    delete[] iv;
    delete[] plain;
    delete[] cipher;
    delete[] dec;
    delete[] feedback;
    return 0;
}
