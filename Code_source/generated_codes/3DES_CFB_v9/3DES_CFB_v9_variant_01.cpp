/ * LLM input variant 1: minimal-boundary * /
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
    int keyA[8], keyB[8], keyC[8];
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
        while (idx < blocks * 2) {
            std::cout << std::hex << std::setw(8) << std::setfill('0') << buf[idx]
                      << " " << std::hex << std::setw(8) << std::setfill('0') << buf[idx + 1]
                      << std::dec << "\n";
            idx = idx + 2;
        }
    };

    std::cout << "Test: single minimal block (8 bytes)\n";
    int plain[2] = {0x01020304, 0x05060708};
    int cipher[2];
    int recovered[2];
    engine.encrypt(plain, cipher, 8, iv);
    engine.decrypt(cipher, recovered, 8, iv);
    std::cout << "Plain : ";   printBlock(plain, 1);
    std::cout << "Cipher: ";   printBlock(cipher, 1);
    std::cout << "Recov : ";   printBlock(recovered, 1);

    return 0;
}
