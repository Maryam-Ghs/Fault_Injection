/* LLM input variant 6: ordered-structured */
#include <iostream>

class TripleDES_OFB {
    int keyMat[3][2];          // three 64‑bit keys (hi, lo)
    int initVec[2];            // IV (hi, lo)
    int feedBack[2];           // current OFB feedback (hi, lo)

    // ----- a tiny, reversible block cipher (placeholder for DES) -----
    void simpleBlock(int &hi, int &lo, int *k) {
        int step = 0;
        while (step < 8) {
            int temp = hi ^ ((k[0] << step) | (k[1] >> (31 - step)));
            hi = lo;
            lo = temp;
            ++step;
        }
    }

    // ----- 3‑DES (E‑D‑E) on a single 64‑bit block -----
    void tripleBlock(int &hi, int &lo) {
        // encrypt with key 0
        simpleBlock(hi, lo, keyMat[0]);
        // decrypt with key 1 (same routine works as our toy cipher)
        simpleBlock(hi, lo, keyMat[1]);
        // encrypt with key 2
        simpleBlock(hi, lo, keyMat[2]);
    }

public:
    void loadKeys(int k0hi, int k0lo, int k1hi, int k1lo, int k2hi, int k2lo) {
        keyMat[0][0] = k0hi; keyMat[0][1] = k0lo;
        keyMat[1][0] = k1hi; keyMat[1][1] = k1lo;
        keyMat[2][0] = k2hi; keyMat[2][1] = k2lo;
    }

    void setIV(int ivhi, int ivlo) {
        initVec[0] = ivhi; initVec[1] = ivlo;
        feedBack[0] = ivhi; feedBack[1] = ivlo;
    }

    // ----- OFB encryption (in‑place) -----
    void encryptStream(int *plain, int *cipher, int blocks) {
        int blk = 0;
        while (blk < blocks) {
            int ksHi = feedBack[0];
            int ksLo = feedBack[1];
            tripleBlock(ksHi, ksLo);               // produce keystream block

            cipher[2 * blk]     = plain[2 * blk]     ^ ksHi;
            cipher[2 * blk + 1] = plain[2 * blk + 1] ^ ksLo;

            feedBack[0] = ksHi;                     // OFB feedback = keystream
            feedBack[1] = ksLo;
            ++blk;
        }
    }

    // ----- OFB decryption (identical to encryption) -----
    void decryptStream(int *cipher, int *plain, int blocks) {
        // reset feedback to the original IV
        feedBack[0] = initVec[0];
        feedBack[1] = initVec[1];

        int blk = 0;
        while (blk < blocks) {
            int ksHi = feedBack[0];
            int ksLo = feedBack[1];
            tripleBlock(ksHi, ksLo);               // same keystream

            plain[2 * blk]     = cipher[2 * blk]     ^ ksHi;
            plain[2 * blk + 1] = cipher[2 * blk + 1] ^ ksLo;

            feedBack[0] = ksHi;
            feedBack[1] = ksLo;
            ++blk;
        }
    }
};

int main() {
    // ----- deterministic, ordered keys and IV -----
    int k0h = 0x11111111; int k0l = 0x11111112;
    int k1h = 0x22222221; int k1l = 0x22222222;
    int k2h = 0x33333331; int k2l = 0x33333332;
    int ivh = 0x44444444; int ivl = 0x44444445;

    // ----- generate a medium‑sized ordered plaintext (8 blocks = 16 ints) -----
    const int blockCount = 8;
    int *plainArr = new int[blockCount * 2];
    for (int i = 0; i < blockCount * 2; ++i) {
        // ascending pattern: each element increases by a fixed step
        plainArr[i] = 0x55555550 + i * 0x1111111;
    }

    // ----- buffers for ciphertext and recovered plaintext -----
    int *cipherArr = new int[blockCount * 2];
    int *recovered = new int[blockCount * 2];

    // ----- set up 3‑DES‑OFB object -----
    TripleDES_OFB engine;
    engine.loadKeys(k0h, k0l, k1h, k1l, k2h, k2l);
    engine.setIV(ivh, ivl);

    // ----- encrypt and then decrypt -----
    engine.encryptStream(plainArr, cipherArr, blockCount);
    engine.decryptStream(cipherArr, recovered, blockCount);

    // ----- display results -----
    std::cout << "Plaintext (hex):\n";
    for (int out = 0; out < blockCount * 2; ++out) {
        std::cout << std::hex << plainArr[out] << " ";
    }
    std::cout << "\n\nCiphertext (hex):\n";
    for (int out = 0; out < blockCount * 2; ++out) {
        std::cout << std::hex << cipherArr[out] << " ";
    }
    std::cout << "\n\nRecovered (hex):\n";
    for (int out = 0; out < blockCount * 2; ++out) {
        std::cout << std::hex << recovered[out] << " ";
    }
    std::cout << std::dec << "\n";

    delete[] plainArr;
    delete[] cipherArr;
    delete[] recovered;
    return 0;
}
