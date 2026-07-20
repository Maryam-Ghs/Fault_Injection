#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */

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
    // ----- adversarial reversed input -----
    // three 8‑byte keys in descending order (worst‑case byte values)
    std::vector<int> keyOne   = {0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88};
    std::vector<int> keyTwo   = {0x87,0x76,0x65,0x54,0x43,0x32,0x21,0x10};
    std::vector<int> keyThree = {0x0F,0x1E,0x2D,0x3C,0x4B,0x5A,0x69,0x78};

    // 8‑byte IV in descending order
    std::vector<int> initVec = {0x80,0x70,0x60,0x50,0x40,0x30,0x20,0x10};

    // plaintext: 64 bytes descending from 0xFF to 0xC0 (worst‑case pattern)
    std::vector<int> plainText = {
        0xFF,0xFE,0xFD,0xFC,0xFB,0xFA,0xF9,0xF8,
        0xF7,0xF6,0xF5,0xF4,0xF3,0xF2,0xF1,0xF0,
        0xEF,0xEE,0xED,0xEC,0xEB,0xEA,0xE9,0xE8,
        0xE7,0xE6,0xE5,0xE4,0xE3,0xE2,0xE1,0xE0,
        0xDF,0xDE,0xDD,0xDC,0xDB,0xDA,0xD9,0xD8,
        0xD7,0xD6,0xD5,0xD4,0xD3,0xD2,0xD1,0xD0,
        0xCF,0xCE,0xCD,0xCC,0xCB,0xCA,0xC9,0xC8,
        0xC7,0xC6,0xC5,0xC4,0xC3,0xC2,0xC1,0xC0
    };

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
