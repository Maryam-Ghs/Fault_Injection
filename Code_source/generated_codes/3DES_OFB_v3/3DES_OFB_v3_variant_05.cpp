#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */

class ToyDES {
public:
    std::vector<int> encrypt(const std::vector<int>& blk,
                             const std::vector<int>& key) {
        std::vector<int> out(8);
        int i = 0;
        while (i < 8) {
            int a = blk[i];
            int b = key[i];
            int rot = ((a << 3) & 0xFF) | ((a >> 5) & 0x07);
            int mixed = rot ^ b;
            out[i] = mixed;
            ++i;
        }
        return out;
    }

    std::vector<int> decrypt(const std::vector<int>& blk,
                             const std::vector<int>& key) {
        std::vector<int> out(8);
        int i = 0;
        while (i < 8) {
            int mixed = blk[i] ^ key[i];
            int a = ((mixed >> 3) & 0x1F) | ((mixed << 5) & 0xE0);
            out[i] = a;
            ++i;
        }
        return out;
    }
};

class TripleDES_OFB {
    ToyDES   desEngine;
    std::vector<int> keyA;
    std::vector<int> keyB;
    std::vector<int> keyC;
    std::vector<int> iv;

public:
    void loadKeys(const std::vector<int>& k1,
                  const std::vector<int>& k2,
                  const std::vector<int>& k3) {
        keyA = k1;
        keyB = k2;
        keyC = k3;
    }

    void setIV(const std::vector<int>& initVec) {
        iv = initVec;
    }

    std::vector<int> tripleEncrypt(const std::vector<int>& block) {
        std::vector<int> stage1 = desEngine.encrypt(block, keyA);
        std::vector<int> stage2 = desEngine.decrypt(stage1, keyB);
        std::vector<int> stage3 = desEngine.encrypt(stage2, keyC);
        return stage3;
    }

    std::vector<int> processOFB(const std::vector<int>& plain) {
        std::vector<int> cipher;
        std::vector<int> feedback = iv;

        int offset = 0;
        while (offset < static_cast<int>(plain.size())) {
            std::vector<int> ks = tripleEncrypt(feedback);

            std::vector<int> chunk(8, 0);
            int j = 0;
            while (j < 8 && (offset + j) < static_cast<int>(plain.size())) {
                chunk[j] = plain[offset + j];
                ++j;
            }

            int k = 0;
            while (k < 8) {
                int ctByte = chunk[k] ^ ks[k];
                cipher.push_back(ctByte);
                ++k;
            }

            feedback = ks;
            offset += 8;
        }
        return cipher;
    }
};

int main() {
    // ----- duplicate‑heavy predefined data -----
    // three 8‑byte keys filled with repeating patterns
    std::vector<int> keyOne   = {0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA};
    std::vector<int> keyTwo   = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};
    std::vector<int> keyThree = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    // 8‑byte IV with all identical bytes
    std::vector<int> initVec = {0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33};

    // plaintext: 16 bytes, all the same value (duplicate‑heavy)
    std::vector<int> plainText = {
        0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
        0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44
    };

    TripleDES_OFB cryptor;
    cryptor.loadKeys(keyOne, keyTwo, keyThree);
    cryptor.setIV(initVec);

    std::vector<int> cipherText = cryptor.processOFB(plainText);

    std::cout << "Ciphertext (hex): ";
    int idx = 0;
    while (idx < static_cast<int>(cipherText.size())) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipherText[idx] & 0xFF) << ' ';
        ++idx;
    }
    std::cout << std::dec << std::endl;
    return 0;
}
