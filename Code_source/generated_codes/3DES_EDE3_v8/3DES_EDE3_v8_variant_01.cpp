#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */

// ---------------------------------------------------------------------
// Helper: convert two signed 32‑bit ints to a vector of 64 bits (0/1)
// ---------------------------------------------------------------------
std::vector<int> bitsFromInts(int high, int low) {
    std::vector<int> bits(64);
    int idx = 0;
    // high part: bits 63..32
    while (idx < 32) {
        bits[idx] = (high >> (31 - idx)) & 1;
        ++idx;
    }
    // low part: bits 31..0
    while (idx < 64) {
        bits[idx] = (low >> (63 - idx)) & 1;
        ++idx;
    }
    return bits;
}

// ---------------------------------------------------------------------
// Helper: convert a vector of 64 bits back to two signed 32‑bit ints
// ---------------------------------------------------------------------
void intsFromBits(const std::vector<int>& bits, int& high, int& low) {
    high = 0;
    low  = 0;
    for (int i = 0; i < 32; ++i) {
        high = (high << 1) | bits[i];
    }
    for (int i = 32; i < 64; ++i) {
        low = (low << 1) | bits[i];
    }
}

// ---------------------------------------------------------------------
// Generic permutation routine (tables are 1‑based)
// ---------------------------------------------------------------------
std::vector<int> permute(const std::vector<int>& src,
                         const std::vector<int>& table) {
    std::vector<int> dst(table.size());
    for (int i = 0; i < (int)table.size(); ++i) {
        dst[i] = src[table[i] - 1];
    }
    return dst;
}

// ---------------------------------------------------------------------
// Left‑circular shift for key schedule (size bits)
// ---------------------------------------------------------------------
void leftRotate(std::vector<int>& keyHalf, int size) {
    std::vector<int> tmp(keyHalf);
    for (int i = 0; i < size; ++i) {
        keyHalf[i] = tmp[(i + 1) % size];
    }
}

// ---------------------------------------------------------------------
// Generate 16 sub‑keys (48 bits each) from a 64‑bit key
// ---------------------------------------------------------------------
std::vector< std::vector<int> > generateSubkeys(const std::vector<int>& key64) {
    static std::vector<int> pc1 = {
        57,49,41,33,25,17,9,
        1,58,50,42,34,26,18,
        10,2,59,51,43,35,27,
        19,11,3,60,52,44,36,
        63,55,47,39,31,23,15,
        7,62,54,46,38,30,22,
        14,6,61,53,45,37,29,
        21,13,5,28,20,12,4
    };
    static std::vector<int> pc2 = {
        14,17,11,24,1,5,
        3,28,15,6,21,10,
        23,19,12,4,26,8,
        16,7,27,20,13,2,
        41,52,31,37,47,55,
        30,40,51,45,33,48,
        44,49,39,56,34,53,
        46,42,50,36,29,32
    };
    static std::vector<int> shifts = {
        1,1,2,2,2,2,2,2,
        1,2,2,2,2,2,2,1
    };

    std::vector<int> permKey = permute(key64, pc1);
    std::vector<int> C(28), D(28);
    for (int i = 0; i < 28; ++i) C[i] = permKey[i];
    for (int i = 0; i < 28; ++i) D[i] = permKey[28 + i];

    std::vector< std::vector<int> > subkeys(16);
    for (int round = 0; round < 16; ++round) {
        for (int s = 0; s < shifts[round]; ++s) {
            leftRotate(C, 28);
            leftRotate(D, 28);
        }
        std::vector<int> CD(56);
        for (int i = 0; i < 28; ++i) CD[i] = C[i];
        for (int i = 0; i < 28; ++i) CD[28 + i] = D[i];
        subkeys[round] = permute(CD, pc2);
    }
    return subkeys;
}

// ---------------------------------------------------------------------
// Expansion (32 → 48 bits)
// ---------------------------------------------------------------------
std::vector<int> expand(const std::vector<int>& halfBlock) {
    static std::vector<int> eTable = {
        32,1,2,3,4,5,
        4,5,6,7,8,9,
        8,9,10,11,12,13,
        12,13,14,15,16,17,
        16,17,18,19,20,21,
        20,21,22,23,24,25,
        24,25,26,27,28,29,
        28,29,30,31,32,1
    };
    return permute(halfBlock, eTable);
}

// ---------------------------------------------------------------------
// S‑Box substitution (48 → 32 bits)
// ---------------------------------------------------------------------
std::vector<int> substitute(const std::vector<int>& input48) {
    static std::vector< std::vector< std::vector<int> > > sBoxes = {
        {
            {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
            {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
            {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
            {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
        },
        {
            {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
            {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
            {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
            {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
        },
        {
            {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
            {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
            {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
            {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
        },
        {
            {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
            {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
            {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
            {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
        },
        {
            {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
            {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
            {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
            {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
        },
        {
            {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
            {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
            {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
            {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
        },
        {
            {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
            {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
            {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
            {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
        },
        {
            {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
            {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
            {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
            {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
        }
    };

    std::vector<int> out32(32);
    for (int i = 0; i < 8; ++i) {
        int row = (input48[i * 6] << 1) | input48[i * 6 + 5];
        int col = (input48[i * 6 + 1] << 3) |
                  (input48[i * 6 + 2] << 2) |
                  (input48[i * 6 + 3] << 1) |
                  (input48[i * 6 + 4]);
        int sVal = sBoxes[i][row][col];
        for (int b = 0; b < 4; ++b) {
            out32[i * 4 + (3 - b)] = (sVal >> b) & 1;
        }
    }
    return out32;
}

// ---------------------------------------------------------------------
// P‑Permutation (32 → 32 bits)
// ---------------------------------------------------------------------
std::vector<int> permuteP(const std::vector<int>& input32) {
    static std::vector<int> pTable = {
        16,7,20,21,
        29,12,28,17,
        1,15,23,26,
        5,18,31,10,
        2,8,24,14,
        32,27,3,9,
        19,13,30,6,
        22,11,4,25
    };
    return permute(input32, pTable);
}

// ---------------------------------------------------------------------
// Feistel function f(R, subKey)
// ---------------------------------------------------------------------
std::vector<int> feistel(const std::vector<int>& rightHalf,
                         const std::vector<int>& subKey) {
    std::vector<int> expanded = expand(rightHalf);
    std::vector<int> xored(48);
    for (int i = 0; i < 48; ++i) {
        xored[i] = expanded[i] ^ subKey[i];
    }
    std::vector<int> sOut = substitute(xored);
    std::vector<int> pOut = permuteP(sOut);
    return pOut;
}

// ---------------------------------------------------------------------
// Single‑DES encryption of a 64‑bit block (EDE mode)
// ---------------------------------------------------------------------
std::vector<int> desEncrypt(const std::vector<int>& plainBlock,
                            const std::vector< std::vector<int> >& subKeys) {
    static std::vector<int> ipTable = {
        58,50,42,34,26,18,10,2,
        60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6,
        64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9,1,
        59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5,
        63,55,47,39,31,23,15,7
    };
    static std::vector<int> fpTable = {
        40,8,48,16,56,24,64,32,
        39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30,
        37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28,
        35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26,
        33,1,41,9,49,17,57,25
    };

    std::vector<int> ip = permute(plainBlock, ipTable);
    std::vector<int> L(32), R(32);
    for (int i = 0; i < 32; ++i) {
        L[i] = ip[i];
        R[i] = ip[32 + i];
    }

    for (int round = 0; round < 16; ++round) {
        std::vector<int> previousL = L;
        L = R;
        std::vector<int> fOut = feistel(R, subKeys[round]);
        std::vector<int> newR(32);
        for (int i = 0; i < 32; ++i) {
            newR[i] = previousL[i] ^ fOut[i];
        }
        R = newR;
    }

    std::vector<int> preOutput(64);
    for (int i = 0; i < 32; ++i) {
        preOutput[i]     = R[i];
        preOutput[32+i] = L[i];
    }

    return permute(preOutput, fpTable);
}

// ---------------------------------------------------------------------
// Triple‑DES EDE3 (Encrypt‑Decrypt‑Encrypt) on a single block
// ---------------------------------------------------------------------
std::vector<int> tripleDesEncrypt(const std::vector<int>& block,
                                  const std::vector<int>& keyA,
                                  const std::vector<int>& keyB,
                                  const std::vector<int>& keyC) {
    std::vector< std::vector<int> > subA = generateSubkeys(keyA);
    std::vector< std::vector<int> > subB = generateSubkeys(keyB);
    std::vector< std::vector<int> > subC = generateSubkeys(keyC);

    std::vector<int> step1 = desEncrypt(block, subA);

    std::vector< std::vector<int> > revB = subB;
    for (int i = 0; i < 8; ++i) {
        std::swap(revB[i], revB[15 - i]);
    }
    std::vector<int> step2 = desEncrypt(step1, revB);

    std::vector<int> finalBlock = desEncrypt(step2, subC);
    return finalBlock;
}

// ---------------------------------------------------------------------
// Print a 64‑bit block as two hex words (high, low)
// ---------------------------------------------------------------------
void printBlock(const std::vector<int>& bits) {
    int hi, lo;
    intsFromBits(bits, hi, lo);
    std::cout << std::hex << std::setw(8) << std::setfill('0') << hi
              << " " << std::setw(8) << std::setfill('0') << lo
              << std::dec << std::endl;
}

// ---------------------------------------------------------------------
// Main – minimal‑size inputs and run 3DES‑EDE3
// ---------------------------------------------------------------------
int main() {
    // Minimal nontrivial plaintext (single 1 bit)
    std::vector< std::pair<int,int> > plains = {
        {0x00000001, 0x00000000}
    };

    // Minimal keys (all zeros)
    std::pair<int,int> key1 = {0x00000000, 0x00000000};
    std::pair<int,int> key2 = {0x00000000, 0x00000000};
    std::pair<int,int> key3 = {0x00000000, 0x00000000};

    std::vector<int> bitsKey1 = bitsFromInts(key1.first, key1.second);
    std::vector<int> bitsKey2 = bitsFromInts(key2.first, key2.second);
    std::vector<int> bitsKey3 = bitsFromInts(key3.first, key3.second);

    std::cout << "3DES‑EDE3 encryption (minimal input)" << std::endl;
    std::cout << "Key A: "; printBlock(bitsKey1);
    std::cout << "Key B: "; printBlock(bitsKey2);
    std::cout << "Key C: "; printBlock(bitsKey3);
    std::cout << std::endl;

    for (size_t idx = 0; idx < plains.size(); ++idx) {
        std::cout << "Plaintext " << idx + 1 << ": ";
        std::vector<int> plainBits = bitsFromInts(plains[idx].first,
                                                  plains[idx].second);
        printBlock(plainBits);

        std::vector<int> cipherBits = tripleDesEncrypt(plainBits,
                                                       bitsKey1,
                                                       bitsKey2,
                                                       bitsKey3);
        std::cout << "Ciphertext: ";
        printBlock(cipherBits);
        std::cout << std::endl;
    }

    return 0;
}
