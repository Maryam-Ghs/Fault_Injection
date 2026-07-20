#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

class A51Cipher {
    int *regA;   // 19 bits
    int *regB;   // 22 bits
    int *regC;   // 23 bits
    int lenA, lenB, lenC;

public:
    A51Cipher(int *keyBits, int *frameBits) {
        lenA = 19; lenB = 22; lenC = 23;
        regA = new int[lenA];
        regB = new int[lenB];
        regC = new int[lenC];

        // ----- load key (64 bits) -----
        int pos = 0;
        for (int i = 0; i < lenA; ++i) regA[i] = keyBits[pos++];
        for (int i = 0; i < lenB; ++i) regB[i] = keyBits[pos++];
        for (int i = 0; i < lenC; ++i) regC[i] = keyBits[pos++];

        // ----- XOR frame bits (22 bits) into registers -----
        for (int i = 0; i < 22; ++i) {
            int fb = frameBits[i];
            // simple XOR into the first cell of each register
            regA[i % lenA] ^= fb;
            regB[i % lenB] ^= fb;
            regC[i % lenC] ^= fb;
        }
    }

    ~A51Cipher() {
        delete[] regA;
        delete[] regB;
        delete[] regC;
    }

    // single step for a given register
    inline int stepA() {
        int t1 = regA[13];
        int t2 = regA[16];
        int t3 = regA[17];
        int t4 = regA[18];
        int feedback = t1 ^ t2 ^ t3 ^ t4;
        for (int i = lenA - 1; i > 0; --i) regA[i] = regA[i - 1];
        regA[0] = feedback;
        return regA[lenA - 1];
    }

    inline int stepB() {
        int t1 = regB[20];
        int t2 = regB[21];
        int feedback = t1 ^ t2;
        for (int i = lenB - 1; i > 0; --i) regB[i] = regB[i - 1];
        regB[0] = feedback;
        return regB[lenB - 1];
    }

    inline int stepC() {
        int t1 = regC[7];
        int t2 = regC[20];
        int t3 = regC[21];
        int t4 = regC[22];
        int feedback = t1 ^ t2 ^ t3 ^ t4;
        for (int i = lenC - 1; i > 0; --i) regC[i] = regC[i - 1];
        regC[0] = feedback;
        return regC[lenC - 1];
    }

    // majority of the three clock bits
    inline int majority() {
        int m1 = regA[8];
        int m2 = regB[10];
        int m3 = regC[10];
        int sum = m1 + m2 + m3;
        return (sum >= 2) ? 1 : 0;
    }

    // generate nbits of keystream, heap‑allocated
    int* generate(int nbits) {
        int *stream = new int[nbits];
        int idx = 0;

        // manual unrolling: produce 4 bits per outer iteration
        while (idx + 3 < nbits) {
            // ---- first bit ----
            int maj = majority();
            int outA = 0, outB = 0, outC = 0;
            if (regA[8] == maj) outA = stepA(); else outA = regA[lenA - 1];
            if (regB[10] == maj) outB = stepB(); else outB = regB[lenB - 1];
            if (regC[10] == maj) outC = stepC(); else outC = regC[lenC - 1];
            stream[idx++] = outA ^ outB ^ outC;

            // ---- second bit ----
            maj = majority();
            if (regA[8] == maj) outA = stepA(); else outA = regA[lenA - 1];
            if (regB[10] == maj) outB = stepB(); else outB = regB[lenB - 1];
            if (regC[10] == maj) outC = stepC(); else outC = regC[lenC - 1];
            stream[idx++] = outA ^ outB ^ outC;

            // ---- third bit ----
            maj = majority();
            if (regA[8] == maj) outA = stepA(); else outA = regA[lenA - 1];
            if (regB[10] == maj) outB = stepB(); else outB = regB[lenB - 1];
            if (regC[10] == maj) outC = stepC(); else outC = regC[lenC - 1];
            stream[idx++] = outA ^ outB ^ outC;

            // ---- fourth bit ----
            maj = majority();
            if (regA[8] == maj) outA = stepA(); else outA = regA[lenA - 1];
            if (regB[10] == maj) outB = stepB(); else outB = regB[lenB - 1];
            if (regC[10] == maj) outC = stepC(); else outC = regC[lenC - 1];
            stream[idx++] = outA ^ outB ^ outC;
        }

        // finish remaining bits (if any)
        while (idx < nbits) {
            int maj = majority();
            int outA = (regA[8] == maj) ? stepA() : regA[lenA - 1];
            int outB = (regB[10] == maj) ? stepB() : regB[lenB - 1];
            int outC = (regC[10] == maj) ? stepC() : regC[lenC - 1];
            stream[idx++] = outA ^ outB ^ outC;
        }

        return stream;
    }
};

int main() {
    // ---- deterministic key (64 bits) ----
    int *key = new int[64];
    for (int i = 0; i < 64; ++i) key[i] = (i % 2); // pattern 0,1,0,1,...

    // ---- deterministic frame number (22 bits) ----
    int *frame = new int[22];
    for (int i = 0; i < 22; ++i) frame[i] = ((i / 2) % 2); // pattern 0,0,1,1,0,0,...

    // ---- create cipher instance ----
    A51Cipher cipher(key, frame);

    // ---- produce a large keystream (10,000 bits) ----
    const int streamSize = 10000;
    int *ks = cipher.generate(streamSize);

    // ---- output ----
    std::cout << "Key (64 bits)   : ";
    for (int i = 0; i < 64; ++i) std::cout << key[i];
    std::cout << "\nFrame (22 bits) : ";
    for (int i = 0; i < 22; ++i) std::cout << frame[i];
    std::cout << "\nKeystream (" << streamSize << ") : ";
    for (int i = 0; i < streamSize; ++i) std::cout << ks[i];
    std::cout << std::endl;

    delete[] key;
    delete[] frame;
    delete[] ks;
    return 0;
}
