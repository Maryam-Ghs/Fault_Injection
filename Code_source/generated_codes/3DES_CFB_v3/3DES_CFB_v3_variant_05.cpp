#include <iostream>
#include <vector>
#include <iomanip>
#include <functional>

/* LLM input variant 5: duplicate-heavy */

// -----------------------------------------------------------------------------
// 3DES in CFB mode – all logic lives inside main() as lambdas
// Uses only int and std::vector<int> for data representation
// -----------------------------------------------------------------------------
int main() {
    // ---------------------------------------------------------
    //  DES tables (int, not const, as required)
    // ---------------------------------------------------------
    static int IP[64] = {
        58,50,42,34,26,18,10,2, 60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6, 64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9,1, 59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5, 63,55,47,39,31,23,15,7
    };
    static int FP[64] = {
        40,8,48,16,56,24,64,32, 39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30, 37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28, 35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26, 33,1,41,9,49,17,57,25
    };
    static int PC1[56] = {
        57,49,41,33,25,17,9, 1,58,50,42,34,26,18,
        10,2,59,51,43,35,27,19,11,3,60,52,44,36,
        63,55,47,39,31,23,15,7,62,54,46,38,30,22,
        14,6,61,53,45,37,29,21,13,5,28,20,12,4
    };
    static int PC2[48] = {
        14,17,11,24,1,5, 3,28,15,6,21,10,
        23,19,12,4,26,8, 16,7,27,20,13,2,
        41,52,31,37,47,55, 30,40,51,45,33,48,
        44,49,39,56,34,53, 46,42,50,36,29,32
    };
    static int E[48] = {
        32,1,2,3,4,5, 4,5,6,7,8,9,
        8,9,10,11,12,13, 12,13,14,15,16,17,
        16,17,18,19,20,21, 20,21,22,23,24,25,
        24,25,26,27,28,29, 28,29,30,31,32,1
    };
    static int P[32] = {
        16,7,20,21, 29,12,28,17,
        1,15,23,26, 5,18,31,10,
        2,8,24,14, 32,27,3,9,
        19,13,30,6, 22,11,4,25
    };
    static int S[8][4][16] = {
        { {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
          {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
          {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
          {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13} },

        { {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
          {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
          {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
          {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9} },

        { {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
          {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
          {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
          {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12} },

        { {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
          {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
          {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
          {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14} },

        { {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
          {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
          {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
          {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3} },

        { {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
          {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
          {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
          {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13} },

        { {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
          {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
          {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
          {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12} }
    };
    static int SHIFTS[16] = {
        1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
    };

    // ---------------------------------------------------------
    // Helper lambdas (all inside main)
    // ---------------------------------------------------------
    auto byteToBits = [&](const std::vector<int>& bytes)->std::vector<int> {
        std::vector<int> bits;
        for (int i = 0; i < (int)bytes.size(); ++i) {
            for (int b = 7; b >= 0; --b) {
                bits.push_back((bytes[i] >> b) & 1);
            }
        }
        return bits;
    };
    auto bitsToBytes = [&](const std::vector<int>& bits)->std::vector<int> {
        std::vector<int> bytes;
        for (int i = 0; i < (int)bits.size(); i += 8) {
            int val = 0;
            for (int j = 0; j < 8; ++j) {
                val = (val << 1) | bits[i + j];
            }
            bytes.push_back(val);
        }
        return bytes;
    };
    auto permute = [&](const std::vector<int>& src, const int* table, int outSize)->std::vector<int> {
        std::vector<int> dst(outSize);
        for (int i = 0; i < outSize; ++i) {
            dst[i] = src[table[i] - 1];
        }
        return dst;
    };
    auto leftRotate28 = [&](int val, int sh)->int {
        int mask = 0x0fffffff;
        int shifted = ((val << sh) | (val >> (28 - sh))) & mask;
        return shifted;
    };
    // ---------------------------------------------------------
    // Key schedule – produces 16 subkeys of 48 bits each
    // ---------------------------------------------------------
    auto genSubKeys = [&](const std::vector<int>& keyBits)->std::vector<std::vector<int>> {
        std::vector<std::vector<int>> subKeys(16);
        std::vector<int> permuted = permute(keyBits, PC1, 56);
        int C = 0, D = 0;
        // pack first 28 bits into C, next 28 into D
        for (int i = 0; i < 28; ++i) {
            C = (C << 1) | permuted[i];
            D = (D << 1) | permuted[i + 28];
        }
        for (int r = 0; r < 16; ++r) {
            C = leftRotate28(C, SHIFTS[r]);
            D = leftRotate28(D, SHIFTS[r]);
            // combine C and D back into 56‑bit vector
            std::vector<int> cd(56);
            for (int i = 27; i >= 0; --i) cd[27 - i] = (C >> i) & 1;
            for (int i = 27; i >= 0; --i) cd[55 - i] = (D >> i) & 1;
            // apply PC2
            subKeys[r] = permute(cd, PC2, 48);
        }
        return subKeys;
    };
    // ---------------------------------------------------------
    // Feistel function f(R, subkey)
    // ---------------------------------------------------------
    auto feistel = [&](const std::vector<int>& rBits,
                       const std::vector<int>& subKey)->std::vector<int> {
        // Expansion E
        std::vector<int> expanded = permute(rBits, E, 48);
        // XOR with subkey (order can be swapped mathematically)
        for (int i = 0; i < 48; ++i) expanded[i] ^= subKey[i];
        // S‑box substitution (8 groups of 6 bits)
        std::vector<int> sOut(32);
        for (int i = 0; i < 8; ++i) {
            int row = (expanded[i*6] << 1) | expanded[i*6 + 5];
            int col = (expanded[i*6 + 1] << 3) |
                      (expanded[i*6 + 2] << 2) |
                      (expanded[i*6 + 3] << 1) |
                      expanded[i*6 + 4];
            int val = S[i][row][col];
            for (int b = 3; b >= 0; --b) {
                sOut[i*4 + (3 - b)] = (val >> b) & 1;
            }
        }
        // Permutation P (order of P after S is mandatory)
        std::vector<int> pOut = permute(sOut, P, 32);
        return pOut;
    };
    // ---------------------------------------------------------
    // DES block encryption (single key)
    // ---------------------------------------------------------
    auto desBlock = [&](const std::vector<int>& block,
                        const std::vector<std::vector<int>>& subKeys,
                        bool encrypt)->std::vector<int> {
        std::vector<int> ip = permute(block, IP, 64);
        std::vector<int> L(32), R(32);
        for (int i = 0; i < 32; ++i) {
            L[i] = ip[i];
            R[i] = ip[i + 32];
        }
        // 16 rounds – direction decided by encrypt flag
        for (int round = 0; round < 16; ++round) {
            int idx = encrypt ? round : 15 - round;
            std::vector<int> fOut = feistel(R, subKeys[idx]);
            std::vector<int> newR(32);
            for (int i = 0; i < 32; ++i) newR[i] = L[i] ^ fOut[i];
            L = R;
            R = newR;
        }
        // Note: final swap (R||L)
        std::vector<int> preFp(64);
        for (int i = 0; i < 32; ++i) {
            preFp[i] = R[i];
            preFp[i + 32] = L[i];
        }
        std::vector<int> fp = permute(preFp, FP, 64);
        return fp;
    };
    // ---------------------------------------------------------
    // Triple‑DES (EDE) – encrypt
    // ---------------------------------------------------------
    auto tripleDesEnc = [&](const std::vector<int>& block,
                            const std::vector<int>& k1,
                            const std::vector<int>& k2,
                            const std::vector<int>& k3)->std::vector<int> {
        std::vector<std::vector<int>> sub1 = genSubKeys(k1);
        std::vector<std::vector<int>> sub2 = genSubKeys(k2);
        std::vector<std::vector<int>> sub3 = genSubKeys(k3);
        std::vector<int> step1 = desBlock(block, sub1, true);
        std::vector<int> step2 = desBlock(step1, sub2, false);
        std::vector<int> step3 = desBlock(step2, sub3, true);
        return step3;
    };
    // ---------------------------------------------------------
    // CFB‑mode encryption (block size = 64 bits)
    // ---------------------------------------------------------
    auto cfbEncrypt = [&](const std::vector<int>& plainBits,
                          const std::vector<int>& ivBits,
                          const std::vector<int>& k1,
                          const std::vector<int>& k2,
                          const std::vector<int>& k3)->std::vector<int> {
        std::vector<int> cipherBits;
        std::vector<int> feedback = ivBits;          // current IV / previous cipher
        int pos = 0;
        while (pos < (int)plainBits.size()) {
            // encrypt feedback with 3DES to obtain keystream
            std::vector<int> ks = tripleDesEnc(feedback, k1, k2, k3);
            // XOR keystream with plaintext block (64 bits)
            std::vector<int> block(64);
            for (int i = 0; i < 64; ++i) {
                block[i] = plainBits[pos + i] ^ ks[i];
            }
            // append ciphertext block
            for (int i = 0; i < 64; ++i) cipherBits.push_back(block[i]);
            // next feedback is the ciphertext block
            feedback = block;
            pos += 64;
        }
        return cipherBits;
    };

    // ---------------------------------------------------------
    // Deterministic test vectors (duplicate-heavy)
    // ---------------------------------------------------------
    std::vector<int> key1Bytes = {0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA};
    std::vector<int> key2Bytes = {0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA};
    std::vector<int> key3Bytes = {0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA};
    std::vector<int> ivBytes   = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    // Plaintext: 16 bytes of the same value (duplicate pattern)
    std::vector<int> plainBytes = {
        0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
        0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55
    };

    // Convert everything to bit vectors
    std::vector<int> k1Bits = byteToBits(key1Bytes);
    std::vector<int> k2Bits = byteToBits(key2Bytes);
    std::vector<int> k3Bits = byteToBits(key3Bytes);
    std::vector<int> ivBits = byteToBits(ivBytes);
    std::vector<int> plainBits = byteToBits(plainBytes);

    // Perform CFB encryption
    std::vector<int> cipherBits = cfbEncrypt(plainBits, ivBits, k1Bits, k2Bits, k3Bits);
    std::vector<int> cipherBytes = bitsToBytes(cipherBits);

    // ---------------------------------------------------------
    // Output ciphertext in hexadecimal
    // ---------------------------------------------------------
    std::cout << "Ciphertext (hex): ";
    for (int i = 0; i < (int)cipherBytes.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipherBytes[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;
    return 0;
}
