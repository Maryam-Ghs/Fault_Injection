/ * LLM input variant 6: ordered-structured * /
#include <iostream>
#include <iomanip>

class TripleDESCFB {
public:
    void setKeys(int k1[8], int k2[8], int k3[8]) {
        int i = 0;
        while (i < 8) {
            keyOne[i] = k1[i];
            keyTwo[i] = k2[i];
            keyThree[i] = k3[i];
            i = i + 1;
        }
    }

    void encrypt(int *plain, int *cipher, int length, int iv[2]) {
        int feed[2];
        feed[0] = iv[0];
        feed[1] = iv[1];

        int pos = 0;
        while (pos < length) {
            int ks[2];
            tripleEncrypt(feed, ks);

            int blockIn[2];
            blockIn[0] = plain[pos / 4];
            blockIn[1] = plain[pos / 4 + 1];

            int out0 = blockIn[0] ^ ks[0];
            int out1 = blockIn[1] ^ ks[1];

            cipher[pos / 4]     = out0;
            cipher[pos / 4 + 1] = out1;

            feed[0] = out0;
            feed[1] = out1;

            pos = pos + 8;
        }
    }

    void decrypt(int *cipher, int *plain, int length, int iv[2]) {
        int feed[2];
        feed[0] = iv[0];
        feed[1] = iv[1];

        int pos = 0;
        while (pos < length) {
            int ks[2];
            tripleEncrypt(feed, ks);

            int blockIn[2];
            blockIn[0] = cipher[pos / 4];
            blockIn[1] = cipher[pos / 4 + 1];

            int out0 = blockIn[0] ^ ks[0];
            int out1 = blockIn[1] ^ ks[1];

            plain[pos / 4]     = out0;
            plain[pos / 4 + 1] = out1;

            feed[0] = blockIn[0];
            feed[1] = blockIn[1];

            pos = pos + 8;
        }
    }

private:
    int keyOne[8];
    int keyTwo[8];
    int keyThree[8];

    void desEncryptBlock(int in[2], int out[2], int key[8]) {
        int left  = in[0];
        int right = in[1];
        int round = 0;
        while (round < 8) {
            int sub = key[round];
            int f = ((right + sub) ^ ((right << 3) | (right >> 29)));
            int newLeft = left ^ f;
            left  = right;
            right = newLeft;
            round = round + 1;
        }
        out[0] = left;
        out[1] = right;
    }

    void desDecryptBlock(int in[2], int out[2], int key[8]) {
        int left  = in[0];
        int right = in[1];
        int round = 7;
        while (round >= 0) {
            int sub = key[round];
            int f = ((left + sub) ^ ((left << 3) | (left >> 29)));
            int newRight = right ^ f;
            right = left;
            left  = newRight;
            round = round - 1;
        }
        out[0] = left;
        out[1] = right;
    }

    void tripleEncrypt(int block[2], int out[2]) {
        int step1[2];
        desEncryptBlock(block, step1, keyOne);
        int step2[2];
        desDecryptBlock(step1, step2, keyTwo);
        desEncryptBlock(step2, out, keyThree);
    }
};

int main() {
    int keyA[8]; int keyB[8]; int keyC[8];
    int i = 0;
    while (i < 8) {
        keyA[i] = i * 0x11111111;
        keyB[i] = i * 0x22222222;
        keyC[i] = i * 0x33333333;
        i = i + 1;
    }

    int iv[2];
    iv[0] = 0x12345678;
    iv[1] = 0x9ABCDEF0;

    TripleDESCFB engine;
    engine.setKeys(keyA, keyB, keyC);

    auto printBlock = [](int *buf, int blocks) {
        int idx = 0;
        while (idx < blocks) {
            std::cout << std::hex << std::setw(8) << std::setfill('0') << buf[idx]
                      << " " << std::hex << std::setw(8) << std::setfill('0') << buf[idx + 1]
                      << std::dec << "\n";
            idx = idx + 2;
        }
    };

    std::cout << "Test 1: zero‑length input\n";

    std::cout << "\nTest 2: single block (incrementing bytes)\n";
    int plain1[2];
    plain1[0] = 0x01020304;
    plain1[1] = 0x05060708;
    int cipher1[2];
    int recovered1[2];
    engine.encrypt(plain1, cipher1, 8, iv);
    engine.decrypt(cipher1, recovered1, 8, iv);
    std::cout << "Plain : "; printBlock(plain1, 1);
    std::cout << "Cipher: "; printBlock(cipher1, 1);
    std::cout << "Recov : "; printBlock(recovered1, 1);

    std::cout << "\nTest 3: two blocks (ordered pattern)\n";
    int plain2[4];
    plain2[0] = 0x11111111;
    plain2[1] = 0x22222222;
    plain2[2] = 0x33333333;
    plain2[3] = 0x44444444;
    int cipher2[4];
    int recovered2[4];
    engine.encrypt(plain2, cipher2, 16, iv);
    engine.decrypt(cipher2, recovered2, 16, iv);
    std::cout << "Plain : "; printBlock(plain2, 2);
    std::cout << "Cipher: "; printBlock(cipher2, 2);
    std::cout << "Recov : "; printBlock(recovered2, 2);

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

    std::cout << "\nTest 5: all‑0xFF block\n";
    int plain4[2];
    plain4[0] = -1;
    plain4[1] = -1;
    int cipher4[2];
    int recovered4[2];
    engine.encrypt(plain4, cipher4, 8, iv);
    engine.decrypt(cipher4, recovered4, 8, iv);
    std::cout << "Plain : "; printBlock(plain4, 1);
    std::cout << "Cipher: "; printBlock(cipher4, 1);
    std::cout << "Recov : "; printBlock(recovered4, 1);

    std::cout << "\nTest 6: eight ordered blocks (ascending pattern)\n";
    int plain5[16];
    i = 0;
    while (i < 16) {
        plain5[i] = (i + 1) * 0x11111111;
        i = i + 1;
    }
    int cipher5[16];
    int recovered5[16];
    engine.encrypt(plain5, cipher5, 64, iv);
    engine.decrypt(cipher5, recovered5, 64, iv);
    std::cout << "Plain : "; printBlock(plain5, 8);
    std::cout << "Cipher: "; printBlock(cipher5, 8);
    std::cout << "Recov : "; printBlock(recovered5, 8);

    return 0;
}
