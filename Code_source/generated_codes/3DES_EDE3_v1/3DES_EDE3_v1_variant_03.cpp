/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <iomanip>

// ------------------------------------------------------------
//  TripleDES (EDE3) – pure integer implementation, step‑by‑step
// ------------------------------------------------------------

class TripleDES {
public:
    // 8‑byte (64‑bit) keys, three of them
    int key1[8];
    int key2[8];
    int key3[8];

    // ----------------------------------------------------------------
    //  Set the three keys (each supplied as 64‑bit integer literals)
    // ----------------------------------------------------------------
    void setKeys(unsigned long long k1, unsigned long long k2, unsigned long long k3) {
        int i;
        for (i = 0; i < 8; ++i) {
            key1[i] = (int)((k1 >> (56 - 8 * i)) & 0xFF);
            key2[i] = (int)((k2 >> (56 - 8 * i)) & 0xFF);
            key3[i] = (int)((k3 >> (56 - 8 * i)) & 0xFF);
        }
    }

    // ----------------------------------------------------------------
    //  Public interface – encrypt a single 64‑bit block (EDE3)
    // ----------------------------------------------------------------
    unsigned long long encryptBlock(unsigned long long plain) {
        unsigned long long step1 = desEncrypt(plain, key1);
        unsigned long long step2 = desDecrypt(step1, key2);
        unsigned long long step3 = desEncrypt(step2, key3);
        return step3;
    }

private:
    // ------------------------------------------------------------
    //  DES core – encrypt one block with a single 8‑byte key
    // ------------------------------------------------------------
    unsigned long long desEncrypt(unsigned long long block, int key[8]) {
        int i;
        // 1) Initial Permutation (IP)
        unsigned long long ip = permute(block, IP, 64);

        // 2) Split into left (L) and right (R) halves
        unsigned int L = (unsigned int)(ip >> 32);
        unsigned int R = (unsigned int)(ip & 0xFFFFFFFFU);

        // 3) Generate the 16 round sub‑keys
        unsigned long long subKeys[16];
        generateSubKeys(key, subKeys);

        // 4) 16 rounds of Feistel function
        for (i = 0; i < 16; ++i) {
            unsigned int previousL = L;
            L = R;
            R = previousL ^ feistel(R, subKeys[i]);
        }

        // 5) Pre‑output: swap halves back
        unsigned long long preoutput = ((unsigned long long)R << 32) | (unsigned long long)L;

        // 6) Final Permutation (FP)
        unsigned long long cipher = permute(preoutput, FP, 64);
        return cipher;
    }

    // ------------------------------------------------------------
    //  DES core – decrypt one block with a single 8‑byte key
    // ------------------------------------------------------------
    unsigned long long desDecrypt(unsigned long long block, int key[8]) {
        int i;
        unsigned long long ip = permute(block, IP, 64);
        unsigned int L = (unsigned int)(ip >> 32);
        unsigned int R = (unsigned int)(ip & 0xFFFFFFFFU);
        unsigned long long subKeys[16];
        generateSubKeys(key, subKeys);

        // rounds in reverse order
        for (i = 15; i >= 0; --i) {
            unsigned int previousL = L;
            L = R;
            R = previousL ^ feistel(R, subKeys[i]);
        }

        unsigned long long preoutput = ((unsigned long long)R << 32) | (unsigned long long)L;
        unsigned long long plain = permute(preoutput, FP, 64);
        return plain;
    }

    // ------------------------------------------------------------
    //  Feistel (F) function – expands, mixes with sub‑key, S‑boxes,
    //  and permutation (P)
    // ------------------------------------------------------------
    unsigned int feistel(unsigned int half, unsigned long long subKey) {
        // 1) Expansion (E) – 32 → 48 bits
        unsigned long long expanded = permute((unsigned long long)half, E, 48);

        // 2) XOR with round sub‑key
        unsigned long long xored = expanded ^ subKey;

        // 3) S‑box substitution (8 groups of 6 bits → 4 bits)
        unsigned int sboxed = 0;
        int s;
        for (s = 0; s < 8; ++s) {
            int sixBits = (int)((xored >> (42 - 6 * s)) & 0x3F);
            int row = ((sixBits & 0x20) >> 4) | (sixBits & 0x01);
            int col = (sixBits >> 1) & 0x0F;
            int sVal = SBOX[s][row * 16 + col];
            sboxed = (sboxed << 4) | (unsigned int)sVal;
        }

        // 4) Permutation (P) – 32 bits
        unsigned int permuted = (unsigned int)permute((unsigned long long)sboxed, P, 32);
        return permuted;
    }

    // ------------------------------------------------------------
    //  Generate 16 sub‑keys (48‑bit each) from the supplied key
    // ------------------------------------------------------------
    void generateSubKeys(int key[8], unsigned long long subKeys[16]) {
        // 1) Apply PC‑1 (56‑bit key)
        unsigned long long permutedKey = permute(packKey(key), PC1, 56);

        // 2) Split into C and D halves (28 bits each)
        unsigned int C = (unsigned int)(permutedKey >> 28);
        unsigned int D = (unsigned int)(permutedKey & 0x0FFFFFFFU);

        // 3) Perform left shifts and PC‑2 for each round
        int i;
        for (i = 0; i < 16; ++i) {
            C = ((C << SHIFTS[i]) | (C >> (28 - SHIFTS[i]))) & 0x0FFFFFFF;
            D = ((D << SHIFTS[i]) | (D >> (28 - SHIFTS[i]))) & 0x0FFFFFFF;
            unsigned long long combined = ((unsigned long long)C << 28) | (unsigned long long)D;
            subKeys[i] = permute(combined, PC2, 48);
        }
    }

    // ------------------------------------------------------------
    //  Pack 8‑byte key array into 64‑bit integer
    // ------------------------------------------------------------
    unsigned long long packKey(int key[8]) {
        int i;
        unsigned long long packed = 0;
        for (i = 0; i < 8; ++i) {
            packed = (packed << 8) | (unsigned long long)(key[i] & 0xFF);
        }
        return packed;
    }

    // ------------------------------------------------------------
    //  Generic permutation routine – source bits are taken from
    //  the most‑significant side (bit 0 = MSB)
    // ------------------------------------------------------------
    unsigned long long permute(unsigned long long src, const int table[], int outSize) {
        int i;
        unsigned long long dst = 0;
        for (i = 0; i < outSize; ++i) {
            int srcPos = table[i] - 1;
            unsigned long long bit = (src >> (64 - srcPos - 1)) & 0x1ULL;
            dst = (dst << 1) | bit;
        }
        return dst;
    }

    // ------------------------------------------------------------
    //  DES tables – all stored as plain int arrays on the stack
    // ------------------------------------------------------------
    // Initial Permutation (IP)
    int IP[64] = {
        58,50,42,34,26,18,10,2,
        60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6,
        64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9,1,
        59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5,
        63,55,47,39,31,23,15,7
    };

    // Final Permutation (FP)
    int FP[64] = {
        40,8,48,16,56,24,64,32,
        39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30,
        37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28,
        35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26,
        33,1,41,9,49,17,57,25
    };

    // Expansion table (E) – 32 → 48 bits
    int E[48] = {
        32,1,2,3,4,5,
        4,5,6,7,8,9,
        8,9,10,11,12,13,
        12,13,14,15,16,17,
        16,17,18,19,20,21,
        20,21,22,23,24,25,
        24,25,26,27,28,29,
        28,29,30,31,32,1
    };

    // Permutation (P) – 32 bits
    int P[32] = {
        16,7,20,21,
        29,12,28,17,
        1,15,23,26,
        5,18,31,10,
        2,8,24,14,
        32,27,3,9,
        19,13,30,6,
        22,11,4,25
    };

    // Permuted Choice 1 (PC‑1) – 64 → 56 bits
    int PC1[56] = {
        57,49,41,33,25,17,9,
        1,58,50,42,34,26,18,
        10,2,59,51,43,35,27,
        19,11,3,60,52,44,36,
        63,55,47,39,31,23,15,
        7,62,54,46,38,30,22,
        14,6,61,53,45,37,29,
        21,13,5,28,20,12,4
    };

    // Permuted Choice 2 (PC‑2) – 56 → 48 bits
    int PC2[48] = {
        14,17,11,24,1,5,
        3,28,15,6,21,10,
        23,19,12,4,26,8,
        16,7,27,20,13,2,
        41,52,31,37,47,55,
        30,40,51,45,33,48,
        44,49,39,56,34,53,
        46,42,50,36,29,32
    };

    // Number of left shifts per round
    int SHIFTS[16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

    // S‑boxes – each 4‑bit output, stored row‑major (16 columns)
    int SBOX[8][64] = {
        {
            14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
            0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
            4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
            15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13
        },
        {
            15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
            3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
            0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
            13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9
        },
        {
            10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
            13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
            13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
            1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12
        },
        {
            7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
            13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
            10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
            3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14
        },
        {
            2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
            14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
            4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
            11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3
        },
        {
            12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
            10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
            9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
            4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13
        },
        {
            4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
            13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
            1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
            6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12
        }
    };

    // ------------------------------------------------------------
    //  Permutation tables for convenience – declared as members
    // ------------------------------------------------------------
    // (already defined above)
};

// ------------------------------------------------------------
//  Helper – pretty‑print a 64‑bit value as hex
// ------------------------------------------------------------
void printHex(const char *label, unsigned long long value) {
    std::cout << label << " = 0x" << std::hex << std::uppercase << std::setw(16)
              << std::setfill('0') << value << std::dec << std::nouppercase << std::endl;
}

// ------------------------------------------------------------
//  Main – deterministic test vectors, no I/O from user
// ------------------------------------------------------------
int main() {
    // 1) Plaintext (64‑bit) – emphasize zeros
    unsigned long long plaintext = 0x0000000000000000ULL;

    // 2) Three keys (each 64‑bit) – zeros, ones, and identity‑like pattern
    unsigned long long k1 = 0x0000000000000000ULL; // all zeros
    unsigned long long k2 = 0xFFFFFFFFFFFFFFFFULL; // all ones
    unsigned long long k3 = 0x1111111111111111ULL; // pattern of ones in low nibble

    // 3) Create TripleDES object and load keys
    TripleDES tdes;
    tdes.setKeys(k1, k2, k3);

    // 4) Encrypt the block
    unsigned long long ciphertext = tdes.encryptBlock(plaintext);

    // 5) Show the deterministic test vector
    printHex("Plaintext ", plaintext);
    printHex("Key 1     ", k1);
    printHex("Key 2     ", k2);
    printHex("Key 3     ", k3);
    printHex("Ciphertext", ciphertext);

    return 0;
}
