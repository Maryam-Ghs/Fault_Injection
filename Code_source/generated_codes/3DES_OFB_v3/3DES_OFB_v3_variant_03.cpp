#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 3: zeros-and-ones */

// Simple toy DES block operation (64‑bit = 8 bytes)
// ONLY FOR DEMONSTRATION – NOT CRYPTOGRAPHICALLY SECURE
class ToyDES {
public:
    // encrypt a single 8‑byte block with a single 8‑byte key
    std::vector<int> encrypt(const std::vector<int>& blk,
                             const std::vector<int>& key) {
        std::vector<int> out(8);
        int i = 0;
        while (i < 8) {
            // split into temporary variables
            int a = blk[i];
            int b = key[i];
            // rotate left 3 bits (mask to keep byte range)
            int rot = ((a << 3) & 0xFF) | ((a >> 5) & 0x07);
            // simple mixing
            int mixed = rot ^ b;
            out[i] = mixed;
            ++i;
        }
        return out;
    }

    // decrypt is the inverse of the toy encrypt (for symmetry)
    std::vector<int> decrypt(const std::vector<int>& blk,
                             const std::vector<int>& key) {
        std::vector<int> out(8);
        int i = 0;
        while (i < 8) {
            int mixed = blk[i] ^ key[i];
            // rotate right 3 bits (inverse of rotate left)
            int a = ((mixed >> 3) & 0x1F) | ((mixed << 5) & 0xE0);
            out[i] = a;
            ++i;
        }
        return out;
    }
};

// Triple‑DES in OFB mode, class‑based implementation
class TripleDES_OFB {
    ToyDES   desEngine;
    std::vector<int> keyA;   // first key
    std::vector<int> keyB;   // second key
    std::vector<int> keyC;   // third key
    std::vector<int> iv;     // initialization vector (8 bytes)

public:
    // set the three 8‑byte keys
    void loadKeys(const std::vector<int>& k1,
                  const std::vector<int>& k2,
                  const std::vector<int>& k3) {
        keyA = k1;
        keyB = k2;
        keyC = k3;
    }

    // set the 8‑byte IV
    void setIV(const std::vector<int>& initVec) {
        iv = initVec;
    }

    // 3‑DES encryption of a block (E‑D‑E)
    std::vector<int> tripleEncrypt(const std::vector<int>& block) {
        // first stage
        std::vector<int> stage1 = desEngine.encrypt(block, keyA);
        // second stage (decrypt with keyB)
        std::vector<int> stage2 = desEngine.decrypt(stage1, keyB);
        // third stage
        std::vector<int> stage3 = desEngine.encrypt(stage2, keyC);
        return stage3;
    }

    // OFB encryption (same routine for decryption)
    std::vector<int> processOFB(const std::vector<int>& plain) {
        // split plain into 8‑byte blocks, pad with zeros if needed
        std::vector<int> cipher;
        std::vector<int> feedback = iv;   // current OFB state

        // work block by block
        int offset = 0;
        while (offset < static_cast<int>(plain.size())) {
            // generate keystream block
            std::vector<int> ks = tripleEncrypt(feedback);

            // take up to 8 bytes from plain
            std::vector<int> chunk(8, 0);
            int j = 0;
            while (j < 8 && (offset + j) < static_cast<int>(plain.size())) {
                chunk[j] = plain[offset + j];
                ++j;
            }

            // XOR plaintext chunk with keystream
            int k = 0;
            while (k < 8) {
                int ctByte = chunk[k] ^ ks[k];
                cipher.push_back(ctByte);
                ++k;
            }

            // next feedback is the keystream (OFB definition)
            feedback = ks;
            offset += 8;
        }
        return cipher;
    }
};

int main() {
    // ----- predefined small data -----
    // three 8‑byte keys emphasizing zeros and ones
    std::vector<int> keyOne   = {0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01};
    std::vector<int> keyTwo   = {0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00};
    std::vector<int> keyThree = {0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01};

    // 8‑byte IV filled with zeros (empty‑like case)
    std::vector<int> initVec = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    // plaintext consisting of alternating zeros and ones (7 bytes, last padded zero)
    std::vector<int> plainText = {0x00,0x01,0x00,0x01,0x00,0x01,0x00};

    // ----- run 3DES‑OFB -----
    TripleDES_OFB cryptor;
    cryptor.loadKeys(keyOne, keyTwo, keyThree);
    cryptor.setIV(initVec);

    std::vector<int> cipherText = cryptor.processOFB(plainText);

    // ----- output ciphertext in hex -----
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
