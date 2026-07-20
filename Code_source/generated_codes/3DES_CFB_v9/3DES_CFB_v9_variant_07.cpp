/* LLM input variant 7: reverse-adversarial */
// version 9 - 3DES in CFB mode, class based, stack arrays only
#include <iostream>
#include <iomanip>

class TripleDESCFB {
public:
    // set three 8‑int keys (each key is 64‑bit => two 32‑bit ints, padded to 8 for demo)
    void setKeys(int k1[8], int k2[8], int k3[8]) {
        int i = 0;
        while (i < 8) {
            keyOne[i] = k1[i];
            keyTwo[i] = k2[i];
            keyThree[i] = k3[i];
            i = i + 1;
        }
    }

    // encrypt using CFB, length is number of bytes (multiple of 8)
    void encrypt(int *plain, int *cipher, int length, int iv[2]) {
        int feed[2];
        feed[0] = iv[0];
        feed[1] = iv[1];

        int pos = 0;
        while (pos < length) {
            int ks[2];
            tripleEncrypt(feed, ks);                     // generate keystream

            int blockIn[2];
            blockIn[0] = plain[pos / 4];
            blockIn[1] = plain[pos / 4 + 1];

            // XOR plaintext with keystream
            int out0 = blockIn[0] ^ ks[0];
            int out1 = blockIn[1] ^ ks[1];

            cipher[pos / 4]     = out0;
            cipher[pos / 4 + 1] = out1;

            // feed becomes the ciphertext just produced
            feed[0] = out0;
            feed[1] = out1;

            pos = pos + 8;
        }
    }

    // decrypt using CFB, length is number of bytes (multiple of 8)
    void decrypt(int *cipher, int *plain, int length, int iv[2]) {
        int feed[2];
        feed[0] = iv[0];
        feed[1] = iv[1];

        int pos = 0;
        while (pos < length) {
            int ks[2];
            tripleEncrypt(feed, ks);                     // generate keystream

            int blockIn[2];
            blockIn[0] = cipher[pos / 4];
            blockIn[1] = cipher[pos / 4 + 1];

            // XOR ciphertext with keystream
            int out0 = blockIn[0] ^ ks[0];
            int out1 = blockIn[1] ^ ks[1];

            plain[pos / 4]     = out0;
            plain[pos / 4 + 1] = out1;

            // feed becomes the ciphertext (not the plaintext)
            feed[0] = blockIn[0];
            feed[1] = blockIn[1];

            pos = pos + 8;
        }
    }

private:
    int keyOne[8];
    int keyTwo[8];
    int keyThree[8];

    // single‑DES encryption (very small Feistel, 8 rounds)
    void desEncryptBlock(int in[2], int out[2], int key[8]) {
        int left  = in[0];
        int right = in[1];
        int round = 0;
        while (round < 8) {
            int sub = key[round];
            // simple round function
            int f = ((right + sub) ^ ((right << 3) | (right >> 29)));
            int newLeft = left ^ f;
            // swap
            left  = right;
            right = newLeft;
            round = round + 1;
        }
        out[0] = left;
        out[1] = right;
    }

    // single‑DES decryption (reverse round order)
    void desDecryptBlock(int in[2], int out[2], int key[8]) {
        int left  = in[0];
        int right = in[1];
        int round = 7;
        while (round >= 0) {
            int sub = key[round];
            int f = ((left + sub) ^ ((left << 3) | (left >> 29)));
            int newRight = right ^ f;
            // swap back
            right = left;
            left  = newRight;
            round = round - 1;
        }
        out[0] = left;
        out[1] = right;
    }

    // triple‑DES (EDE) on a 64‑bit block
    void tripleEncrypt(int block[2], int out[2]) {
        int step1[2];
        desEncryptBlock(block, step1, keyOne);
        int step2[2];
        desDecryptBlock(step1, step2, keyTwo);
        desEncryptBlock(step2, out, keyThree);
    }
};

int main() {
    // reverse‑ordered, adversarial test data

    // keys (descending patterns)
    int keyA[8]; int keyB[8]; int keyC[8];
    int i = 0;
    while (i < 8) {
        keyA[i] = (7 - i) * 0x11111111;
        keyB[i] = (7 - i) * 0x22222222;
        keyC[i] = (7 - i) * 0x33333333;
        i = i + 1;
    }

    // IV (worst‑case extremes)
    int iv[2];
    iv[0] = 0xFFFFFFFF;
    iv[1] = 0x00000000;

    TripleDESCFB engine;
    engine.setKeys(keyA, keyB, keyC);

    // helper to print 64‑bit block as two hex ints
    auto printBlock = [](int *buf, int blocks) {
        int idx = 0;
        while (idx < blocks) {
            std::cout << std::hex << std::setw(8) << std::setfill('0') << buf[idx]
                      << " " << std::hex << std::setw(8) << std::setfill('0') << buf[idx + 1]
                      << std::dec << "\n";
            idx = idx + 2;
        }
    };

    // 1) zero‑length (nothing to do, just show message)
    std::cout << "Test 1: zero‑length input\n";

    // 2) single block, descending bytes
    std::cout << "\nTest 2: single block (descending bytes)\n";
    int plain1[2];
    plain1[0] = 0xF0F0F0F0;
    plain1[1] = 0x0F0F0F0F;
    int cipher1[2];
    int recovered1[2];
    engine.encrypt(plain1, cipher1, 8, iv);
    engine.decrypt(cipher1, recovered1, 8, iv);
    std::cout << "Plain : "; printBlock(plain1, 1);
    std::cout << "Cipher: "; printBlock(cipher1, 1);
    std::cout << "Recov : "; printBlock(recovered1, 1);

    // 3) four blocks, reverse‑ordered pattern data
    std::cout << "\nTest 3: four blocks (reverse‑ordered pattern)\n";
    int plain2[8];
    plain2[0] = 0xFFFFFFFF; // block 1 high
    plain2[1] = 0xEEEEEEEE;
    plain2[2] = 0xDDDDDDDD; // block 2
    plain2[3] = 0xCCCCCCCC;
    plain2[4] = 0xBBBBBBBB; // block 3
    plain2[5] = 0xAAAAAAAA;
    plain2[6] = 0x99999999; // block 4 low
    plain2[7] = 0x88888888;
    int cipher2[8];
    int recovered2[8];
    engine.encrypt(plain2, cipher2, 32, iv);
    engine.decrypt(cipher2, recovered2, 32, iv);
    std::cout << "Plain : "; printBlock(plain2, 4);
    std::cout << "Cipher: "; printBlock(cipher2, 4);
    std::cout << "Recov : "; printBlock(recovered2, 4);

    // 4) all‑zero block
    std::cout << "\nTest 4: all‑zero block\n";
    int plain3[2];
    plain3[0] = 0x00000000;
    plain3[1] = 0x00000000;
    int cipher3[2];
    int recovered3[2];
    engine.encrypt(plain3, cipher3, 8, iv);
    engine.decrypt(cipher3, recovered3, 8, iv);
    std::cout << "Plain : "; printBlock(plain3, 1);
    std::cout << "Cipher: "; printBlock(cipher3, 1);
    std::cout << "Recov : "; printBlock(recovered3, 1);

    // 5) all‑0xFF block (signed negative ints)
    std::cout << "\nTest 5: all‑0xFF block\n";
    int plain4[2];
    plain4[0] = -1;               // 0xFFFFFFFF as signed int
    plain4[1] = -1;
    int cipher4[2];
    int recovered4[2];
    engine.encrypt(plain4, cipher4, 8, iv);
    engine.decrypt(cipher4, recovered4, 8, iv);
    std::cout << "Plain : "; printBlock(plain4, 1);
    std::cout << "Cipher: "; printBlock(cipher4, 1);
    std::cout << "Recov : "; printBlock(recovered4, 1);

    return 0;
}
