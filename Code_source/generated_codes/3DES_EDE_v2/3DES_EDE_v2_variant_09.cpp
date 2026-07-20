/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <iomanip>

/* ------------------------------------------------------------------
 *  Helper functions for bit manipulation on signed int arrays
 * ------------------------------------------------------------------ */
int getBit(const int* bits, int pos)            // pos = 0 … 63
{
    return bits[pos];
}

void setBit(int* bits, int pos, int val)        // val = 0 or 1
{
    bits[pos] = (val & 1);
}

/* ------------------------------------------------------------------
 *  Fixed DES tables (all values are plain ints, no const)
 * ------------------------------------------------------------------ */
int IP[64] = {
     58, 50, 42, 34, 26, 18, 10,  2,
     60, 52, 44, 36, 28, 20, 12,  4,
     62, 54, 46, 38, 30, 22, 14,  6,
     64, 56, 48, 40, 32, 24, 16,  8,
     57, 49, 41, 33, 25, 17,  9,  1,
     59, 51, 43, 35, 27, 19, 11,  3,
     61, 53, 45, 37, 29, 21, 13,  5,
     63, 55, 47, 39, 31, 23, 15,  7
};

int FP[64] = {
     40,  8, 48, 16, 56, 24, 64, 32,
     39,  7, 47, 15, 55, 23, 63, 31,
     38,  6, 46, 14, 54, 22, 62, 30,
     37,  5, 45, 13, 53, 21, 61, 29,
     36,  4, 44, 12, 52, 20, 60, 28,
     35,  3, 43, 11, 51, 19, 59, 27,
     34,  2, 42, 10, 50, 18, 58, 26,
     33,  1, 41,  9, 49, 17, 57, 25
};

int E[48] = {
     32,  1,  2,  3,  4,  5,
      4,  5,  6,  7,  8,  9,
      8,  9, 10, 11, 12, 13,
     12, 13, 14, 15, 16, 17,
     16, 17, 18, 19, 20, 21,
     20, 21, 22, 23, 24, 25,
     24, 25, 26, 27, 28, 29,
     28, 29, 30, 31, 32,  1
};

int P[32] = {
     16,  7, 20, 21,
     29, 12, 28, 17,
      1, 15, 23, 26,
      5, 18, 31, 10,
      2,  8, 24, 14,
     32, 27,  3,  9,
     19, 13, 30,  6,
     22, 11,  4, 25
};

int SBOX[8][4][16] = {
    {   // S1
        {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
        {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
        {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
        {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
    },
    {   // S2
        {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
        {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
        {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
        {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
    },
    {   // S3
        {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
        {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
        {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
        {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
    },
    {   // S4
        {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
        {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
        {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
        {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
    },
    {   // S5
        {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
        {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
        {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
        {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
    },
    {   // S6
        {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
        {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
        {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
        {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
    },
    {   // S7
        {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
        {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
        {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
        {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
    },
    {   // S8
        {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
        {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
        {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
        {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
    }
};

int PC1[56] = {
     57, 49, 41, 33, 25, 17,  9,
      1, 58, 50, 42, 34, 26, 18,
     10,  2, 59, 51, 43, 35, 27,
     19, 11,  3, 60, 52, 44, 36,
     63, 55, 47, 39, 31, 23, 15,
      7, 62, 54, 46, 38, 30, 22,
     14,  6, 61, 53, 45, 37, 29,
     21, 13,  5, 28, 20, 12,  4
};

int PC2[48] = {
     14, 17, 11, 24,  1,  5,
      3, 28, 15,  6, 21, 10,
     23, 19, 12,  4, 26,  8,
     16,  7, 27, 20, 13,  2,
     41, 52, 31, 37, 47, 55,
     30, 40, 51, 45, 33, 48,
     44, 49, 39, 56, 34, 53,
     46, 42, 50, 36, 29, 32
};

int SHIFT_SCHEDULE[16] = {
    1, 1, 2, 2, 2, 2, 2, 2,
    1, 2, 2, 2, 2, 2, 2, 1
};

/* ------------------------------------------------------------------
 *  Class implementing a single DES engine
 * ------------------------------------------------------------------ */
class DES
{
public:
    // Constructor receives an 8‑byte key (as int[8])
    DES(const int* keyBytes)
    {
        // Allocate storage for 16 sub‑keys, each 48 bits
        subKeys = new int*[16];
        int i = 0;
        while (i < 16)
        {
            subKeys[i] = new int[48];
            i = i + 1;
        }
        // Build sub‑keys from the supplied key
        buildSubKeys(keyBytes);
    }

    ~DES()
    {
        int i = 0;
        while (i < 16)
        {
            delete[] subKeys[i];
            i = i + 1;
        }
        delete[] subKeys;
    }

    // Encrypt a single 8‑byte block (input and output are int[8])
    void encryptBlock(const int* plain, int* cipher) const
    {
        int block[64];
        int i = 0;
        while (i < 64)
        {
            block[i] = 0;
            i = i + 1;
        }

        // ---- Initial permutation (IP) ----
        i = 0;
        while (i < 64)
        {
            int srcPos = IP[i] - 1;               // table is 1‑based
            int byteIdx = srcPos / 8;
            int bitIdx  = srcPos % 8;
            int bitVal  = (plain[byteIdx] >> (7 - bitIdx)) & 1;
            block[i] = bitVal;
            i = i + 1;
        }

        // Split into left (L) and right (R) halves
        int L[32], R[32];
        i = 0;
        while (i < 32)
        {
            L[i] = block[i];
            R[i] = block[i + 32];
            i = i + 1;
        }

        // ---- 16 rounds ----
        int round = 0;
        while (round < 16)
        {
            // Compute f(R, subKey[round])
            int fOut[32];
            fFunction(R, subKeys[round], fOut);

            // New left = old right
            int newL[32];
            i = 0;
            while (i < 32)
            {
                newL[i] = R[i];
                i = i + 1;
            }

            // New right = old left XOR fOut
            int newR[32];
            i = 0;
            while (i < 32)
            {
                newR[i] = L[i] ^ fOut[i];
                i = i + 1;
            }

            // Prepare for next round
            i = 0;
            while (i < 32)
            {
                L[i] = newL[i];
                R[i] = newR[i];
                i = i + 1;
            }

            round = round + 1;
        }

        // ---- Pre‑output (R||L) ----
        i = 0;
        while (i < 32)
        {
            block[i]     = R[i];
            block[i + 32]= L[i];
            i = i + 1;
        }

        // ---- Final permutation (FP) ----
        i = 0;
        while (i < 64)
        {
            int srcPos = FP[i] - 1;
            int bitVal = block[srcPos];
            int byteIdx = i / 8;
            int bitIdx  = i % 8;
            cipher[byteIdx] = (cipher[byteIdx] & ~(1 << (7 - bitIdx))) |
                               (bitVal << (7 - bitIdx));
            i = i + 1;
        }
    }

    // Decrypt a single 8‑byte block (input and output are int[8])
    void decryptBlock(const int* cipher, int* plain) const
    {
        // Decryption is the same as encryption but sub‑keys are used in reverse order
        int block[64];
        int i = 0;
        while (i < 64)
        {
            block[i] = 0;
            i = i + 1;
        }

        // ---- Initial permutation (IP) ----
        i = 0;
        while (i < 64)
        {
            int srcPos = IP[i] - 1;
            int byteIdx = srcPos / 8;
            int bitIdx  = srcPos % 8;
            int bitVal  = (cipher[byteIdx] >> (7 - bitIdx)) & 1;
            block[i] = bitVal;
            i = i + 1;
        }

        // Split into left (L) and right (R)
        int L[32], R[32];
        i = 0;
        while (i < 32)
        {
            L[i] = block[i];
            R[i] = block[i + 32];
            i = i + 1;
        }

        // ---- 16 rounds (reverse order) ----
        int round = 15;
        while (round >= 0)
        {
            // Compute f(R, subKey[round])
            int fOut[32];
            fFunction(R, subKeys[round], fOut);

            // New left = old right
            int newL[32];
            i = 0;
            while (i < 32)
            {
                newL[i] = R[i];
                i = i + 1;
            }

            // New right = old left XOR fOut
            int newR[32];
            i = 0;
            while (i < 32)
            {
                newR[i] = L[i] ^ fOut[i];
                i = i + 1;
            }

            // Prepare for next round
            i = 0;
            while (i < 32)
            {
                L[i] = newL[i];
                R[i] = newR[i];
                i = i + 1;
            }

            round = round - 1;
        }

        // ---- Pre‑output (R||L) ----
        i = 0;
        while (i < 32)
        {
            block[i]     = R[i];
            block[i + 32]= L[i];
            i = i + 1;
        }

        // ---- Final permutation (FP) ----
        i = 0;
        while (i < 64)
        {
            int srcPos = FP[i] - 1;
            int bitVal = block[srcPos];
            int byteIdx = i / 8;
            int bitIdx  = i % 8;
            plain[byteIdx] = (plain[byteIdx] & ~(1 << (7 - bitIdx))) |
                             (bitVal << (7 - bitIdx));
            i = i + 1;
        }
    }

private:
    int** subKeys;   // 16 × 48 bits

    // ------------------------------------------------------------------
    //  Build all 16 sub‑keys from the original 64‑bit key
    // ------------------------------------------------------------------
    void buildSubKeys(const int* keyBytes)
    {
        // Convert key bytes to a 56‑bit array (PC‑1)
        int key56[56];
        int i = 0;
        while (i < 56)
        {
            int srcPos = PC1[i] - 1;
            int byteIdx = srcPos / 8;
            int bitIdx  = srcPos % 8;
            int bitVal  = (keyBytes[byteIdx] >> (7 - bitIdx)) & 1;
            key56[i] = bitVal;
            i = i + 1;
        }

        // Split into C and D (each 28 bits)
        int C[28], D[28];
        i = 0;
        while (i < 28)
        {
            C[i] = key56[i];
            D[i] = key56[i + 28];
            i = i + 1;
        }

        // Generate 16 sub‑keys
        int round = 0;
        while (round < 16)
        {
            // Left‑shift according to schedule
            int shift = SHIFT_SCHEDULE[round];
            // Shift C
            int tempC[28];
            i = 0;
            while (i < 28)
            {
                int src = (i + shift) % 28;
                tempC[i] = C[src];
                i = i + 1;
            }
            i = 0;
            while (i < 28)
            {
                C[i] = tempC[i];
                i = i + 1;
            }
            // Shift D
            int tempD[28];
            i = 0;
            while (i < 28)
            {
                int src = (i + shift) % 28;
                tempD[i] = D[src];
                i = i + 1;
            }
            i = 0;
            while (i < 28)
            {
                D[i] = tempD[i];
                i = i + 1;
            }

            // Combine C and D into 56‑bit key
            int CD56[56];
            i = 0;
            while (i < 28)
            {
                CD56[i] = C[i];
                CD56[i + 28] = D[i];
                i = i + 1;
            }

            // Apply PC‑2 to obtain the 48‑bit sub‑key
            i = 0;
            while (i < 48)
            {
                int srcPos = PC2[i] - 1;
                subKeys[round][i] = CD56[srcPos];
                i = i + 1;
            }

            round = round + 1;
        }
    }

    // ------------------------------------------------------------------
    //  The DES f‑function (R is 32 bits, subKey is 48 bits, out is 32 bits)
    // ------------------------------------------------------------------
    void fFunction(const int* R, const int* subKey, int* out) const
    {
        // Expansion from 32 to 48 bits
        int expanded[48];
        int i = 0;
        while (i < 48)
        {
            int srcPos = E[i] - 1;
            expanded[i] = R[srcPos];
            i = i + 1;
        }

        // XOR with sub‑key
        i = 0;
        while (i < 48)
        {
            expanded[i] ^= subKey[i];
            i = i + 1;
        }

        // S‑box substitution (produces 32 bits)
        int sOut[32];
        i = 0;
        while (i < 8)
        {
            // Take 6 bits for the i‑th S‑box
            int b0 = expanded[i * 6];
            int b5 = expanded[i * 6 + 5];
            int row = (b0 << 1) | b5;
            int b1 = expanded[i * 6 + 1];
            int b2 = expanded[i * 6 + 2];
            int b3 = expanded[i * 6 + 3];
            int b4 = expanded[i * 6 + 4];
            int col = (b1 << 3) | (b2 << 2) | (b3 << 1) | b4;
            int sVal = SBOX[i][row][col];   // 0 … 15

            // Convert the 4‑bit sVal to bits
            int j = 0;
            while (j < 4)
            {
                sOut[i * 4 + (3 - j)] = (sVal >> j) & 1;
                j = j + 1;
            }
            i = i + 1;
        }

        // Permutation P
        i = 0;
        while (i < 32)
        {
            int srcPos = P[i] - 1;
            out[i] = sOut[srcPos];
            i = i + 1;
        }
    }
};

/* ------------------------------------------------------------------
 *  Triple DES (EDE) – combines three DES instances
 * ------------------------------------------------------------------ */
class TripleDES
{
public:
    TripleDES(const int* key1, const int* key2, const int* key3)
    {
        des1 = new DES(key1);
        des2 = new DES(key2);
        des3 = new DES(key3);
    }

    ~TripleDES()
    {
        delete des1;
        delete des2;
        delete des3;
    }

    // Encrypt 8‑byte block
    void encrypt(const int* plain, int* cipher) const
    {
        int tmp1[8] = {0,0,0,0,0,0,0,0};
        int tmp2[8] = {0,0,0,0,0,0,0,0};

        des1->encryptBlock(plain, tmp1);
        des2->decryptBlock(tmp1, tmp2);
        des3->encryptBlock(tmp2, cipher);
    }

    // Decrypt 8‑byte block
    void decrypt(const int* cipher, int* plain) const
    {
        int tmp1[8] = {0,0,0,0,0,0,0,0};
        int tmp2[8] = {0,0,0,0,0,0,0,0};

        des3->decryptBlock(cipher, tmp1);
        des2->encryptBlock(tmp1, tmp2);
        des1->decryptBlock(tmp2, plain);
    }

private:
    DES* des1;
    DES* des2;
    DES* des3;
};

/* ------------------------------------------------------------------
 *  Main – internal test vectors, heap allocation, printing results
 * ------------------------------------------------------------------ */
int main()
{
    // ------------------------------------------------------------------
    // 1. Define a single 8‑byte plaintext block (heap allocated)
    // ------------------------------------------------------------------
    int* plainBlock = new int[8];
    int i = 0;
    while (i < 8)
    {
        plainBlock[i] = 0x00;   // initialise with zeros
        i = i + 1;
    }
    // Deterministic pseudo‑random pattern for plaintext
    int baseP = 0xA5;
    i = 0;
    while (i < 8)
    {
        plainBlock[i] = ((baseP + i * 0x33) ^ 0x5C) & 0xFF;
        i = i + 1;
    }

    // ------------------------------------------------------------------
    // 2. Define three 8‑byte keys (heap allocated)
    // ------------------------------------------------------------------
    int* keyA = new int[8];
    int* keyB = new int[8];
    int* keyC = new int[8];
    i = 0;
    while (i < 8)
    {
        keyA[i] = 0x00;
        keyB[i] = 0x00;
        keyC[i] = 0x00;
        i = i + 1;
    }
    // Key A pattern
    int baseA = 0x12;
    i = 0;
    while (i < 8)
    {
        keyA[i] = ((baseA + i * 0x47) ^ 0x9B) & 0xFF;
        i = i + 1;
    }
    // Key B pattern
    int baseB = 0x34;
    i = 0;
    while (i < 8)
    {
        keyB[i] = ((baseB + i * 0x59) ^ 0x6D) & 0xFF;
        i = i + 1;
    }
    // Key C pattern
    int baseC = 0x56;
    i = 0;
    while (i < 8)
    {
        keyC[i] = ((baseC + i * 0x71) ^ 0x2A) & 0xFF;
        i = i + 1;
    }

    // ------------------------------------------------------------------
    // 3. Create TripleDES object
    // ------------------------------------------------------------------
    TripleDES* tdes = new TripleDES(keyA, keyB, keyC);

    // ------------------------------------------------------------------
    // 4. Allocate buffer for ciphertext
    // ------------------------------------------------------------------
    int* cipherBlock = new int[8];
    i = 0;
    while (i < 8)
    {
        cipherBlock[i] = 0x00;
        i = i + 1;
    }

    // ------------------------------------------------------------------
    // 5. Perform encryption
    // ------------------------------------------------------------------
    tdes->encrypt(plainBlock, cipherBlock);

    // ------------------------------------------------------------------
    // 6. Print plaintext and ciphertext (hexadecimal)
    // ------------------------------------------------------------------
    std::cout << "Plaintext : ";
    i = 0;
    while (i < 8)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (plainBlock[i] & 0xFF) << " ";
        i = i + 1;
    }
    std::cout << std::dec << std::endl;

    std::cout << "Ciphertext: ";
    i = 0;
    while (i < 8)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipherBlock[i] & 0xFF) << " ";
        i = i + 1;
    }
    std::cout << std::dec << std::endl;

    // ------------------------------------------------------------------
    // 7. Decrypt to verify correctness
    // ------------------------------------------------------------------
    int* recovered = new int[8];
    i = 0;
    while (i < 8)
    {
        recovered[i] = 0x00;
        i = i + 1;
    }
    tdes->decrypt(cipherBlock, recovered);

    std::cout << "Recovered : ";
    i = 0;
    while (i < 8)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (recovered[i] & 0xFF) << " ";
        i = i + 1;
    }
    std::cout << std::dec << std::endl;

    // ------------------------------------------------------------------
    // 8. Clean up heap memory
    // ------------------------------------------------------------------
    delete[] plainBlock;
    delete[] keyA;
    delete[] keyB;
    delete[] keyC;
    delete[] cipherBlock;
    delete[] recovered;
    delete tdes;

    return 0;
}
