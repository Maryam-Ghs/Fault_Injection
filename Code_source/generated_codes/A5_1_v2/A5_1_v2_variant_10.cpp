/* LLM input variant 10: large-safe-stress */
#include <iostream>
#include <iomanip>

class A51Cipher {
public:
    // Register lengths
    int lenA, lenB, lenC;
    // Registers allocated on the heap
    int *regA;
    int *regB;
    int *regC;

    A51Cipher() {
        // lengths as per A5/1 specification
        lenA = 19;
        lenB = 22;
        lenC = 23;
        regA = new int[lenA];
        regB = new int[lenB];
        regC = new int[lenC];
        // initialise to zero
        for (int i = 0; i < lenA; ++i) regA[i] = 0;
        for (int i = 0; i < lenB; ++i) regB[i] = 0;
        for (int i = 0; i < lenC; ++i) regC[i] = 0;
    }

    ~A51Cipher() {
        delete[] regA;
        delete[] regB;
        delete[] regC;
    }

    // -----------------------------------------------------------------
    // Load a 64‑bit key (bits supplied as an int array of 0/1 values)
    // -----------------------------------------------------------------
    void loadKey(const int *keyBits) {
        for (int i = 0; i < 64; ++i) {
            // compute feedback for each register using temporary variables
            int fbA = (regA[13] ^ regA[16] ^ regA[17] ^ regA[18]) & 1;
            int fbB = (regB[20] ^ regB[21]) & 1;
            int fbC = (regC[7]  ^ regC[20] ^ regC[21] ^ regC[22]) & 1;

            // shift left (bit 0 becomes new LSB)
            for (int j = lenA - 1; j > 0; --j) regA[j] = regA[j - 1];
            for (int j = lenB - 1; j > 0; --j) regB[j] = regB[j - 1];
            for (int j = lenC - 1; j > 0; --j) regC[j] = regC[j - 1];

            // insert new bit = feedback XOR key bit
            regA[0] = fbA ^ keyBits[i];
            regB[0] = fbB ^ keyBits[i];
            regC[0] = fbC ^ keyBits[i];
        }
    }

    // -----------------------------------------------------------------
    // Load a 22‑bit frame number (bits supplied as an int array of 0/1)
    // -----------------------------------------------------------------
    void loadFrame(const int *frameBits) {
        for (int i = 0; i < 22; ++i) {
            int fbA = (regA[13] ^ regA[16] ^ regA[17] ^ regA[18]) & 1;
            int fbB = (regB[20] ^ regB[21]) & 1;
            int fbC = (regC[7]  ^ regC[20] ^ regC[21] ^ regC[22]) & 1;

            for (int j = lenA - 1; j > 0; --j) regA[j] = regA[j - 1];
            for (int j = lenB - 1; j > 0; --j) regB[j] = regB[j - 1];
            for (int j = lenC - 1; j > 0; --j) regC[j] = regC[j - 1];

            regA[0] = fbA ^ frameBits[i];
            regB[0] = fbB ^ frameBits[i];
            regC[0] = fbC ^ frameBits[i];
        }
    }

    // -----------------------------------------------------------------
    // Perform 100 warm‑up clocks with majority rule (no output)
    // -----------------------------------------------------------------
    void warmup() {
        for (int w = 0; w < 100; ++w) {
            // clocking bits
            int clkA = regA[8];
            int clkB = regB[10];
            int clkC = regC[10];
            // majority of the three bits
            int maj = (clkA + clkB + clkC) >= 2 ? 1 : 0;

            // conditionally clock each register
            if (clkA == maj) clockRegA();
            if (clkB == maj) clockRegB();
            if (clkC == maj) clockRegC();
        }
    }

    // -----------------------------------------------------------------
    // Generate a single keystream bit (majority‑clocked)
    // -----------------------------------------------------------------
    int genBit() {
        int clkA = regA[8];
        int clkB = regB[10];
        int clkC = regC[10];
        int maj = (clkA + clkB + clkC) >= 2 ? 1 : 0;

        if (clkA == maj) clockRegA();
        if (clkB == maj) clockRegB();
        if (clkC == maj) clockRegC();

        // output bit is XOR of the last bits of each register
        int out = (regA[lenA - 1] ^ regB[lenB - 1] ^ regC[lenC - 1]) & 1;
        return out;
    }

    // -----------------------------------------------------------------
    // Produce 64 keystream bits, manually unrolled eight at a time
    // -----------------------------------------------------------------
    void produceKeystream(int *outBits) {
        warmup();                     // discard initial transient

        // unrolled loop: 8 bits per iteration
        for (int i = 0; i < 64; i += 8) {
            int t0 = genBit(); outBits[i + 0] = t0;
            int t1 = genBit(); outBits[i + 1] = t1;
            int t2 = genBit(); outBits[i + 2] = t2;
            int t3 = genBit(); outBits[i + 3] = t3;
            int t4 = genBit(); outBits[i + 4] = t4;
            int t5 = genBit(); outBits[i + 5] = t5;
            int t6 = genBit(); outBits[i + 6] = t6;
            int t7 = genBit(); outBits[i + 7] = t7;
        }
    }

private:
    // -----------------------------------------------------------------
    // Individual register clock functions (feedback computed using temps)
    // -----------------------------------------------------------------
    void clockRegA() {
        int fb = (regA[13] ^ regA[16] ^ regA[17] ^ regA[18]) & 1;
        for (int j = lenA - 1; j > 0; --j) regA[j] = regA[j - 1];
        regA[0] = fb;
    }

    void clockRegB() {
        int fb = (regB[20] ^ regB[21]) & 1;
        for (int j = lenB - 1; j > 0; --j) regB[j] = regB[j - 1];
        regB[0] = fb;
    }

    void clockRegC() {
        int fb = (regC[7] ^ regC[20] ^ regC[21] ^ regC[22]) & 1;
        for (int j = lenC - 1; j > 0; --j) regC[j] = regC[j - 1];
        regC[0] = fb;
    }
};

// ---------------------------------------------------------------------
// Helper: convert 64‑bit array to hex string for printing
// ---------------------------------------------------------------------
void printHex(const int *bits, int count) {
    unsigned int accumulator = 0;
    for (int i = 0; i < count; ++i) {
        accumulator = (accumulator << 1) | (bits[i] & 1);
        // flush every 4 bits
        if ((i & 3) == 3) {
            std::cout << std::hex << accumulator;
            accumulator = 0;
        }
    }
    std::cout << std::dec << std::endl;
}

// ---------------------------------------------------------------------
// Deterministic test vectors (key = alternating 1/0, frame = 0/1 pattern)
// ---------------------------------------------------------------------
void fillTestVectors(int *key, int *frame) {
    for (int i = 0; i < 64; ++i) key[i] = i & 1;          // 101010...
    for (int i = 0; i < 22; ++i) frame[i] = (i & 1) ^ 1; // 010101...
}

// ---------------------------------------------------------------------
// Main entry point
// ---------------------------------------------------------------------
int main() {
    const int ITER = 1000; // large but safe number of repetitions

    // allocate test vectors on the heap
    int *keyVec = new int[64];
    int *frameVec = new int[22];
    int *ksBits = new int[64];
    int *allKeystream = new int[64 * ITER];

    fillTestVectors(keyVec, frameVec);

    for (int i = 0; i < ITER; ++i) {
        A51Cipher cipher;
        cipher.loadKey(keyVec);
        cipher.loadFrame(frameVec);
        cipher.produceKeystream(ksBits);
        // copy into the large buffer
        for (int j = 0; j < 64; ++j) {
            allKeystream[i * 64 + j] = ksBits[j];
        }
    }

    std::cout << "A5/1 keystream (hex) for " << ITER << " iterations: ";
    printHex(allKeystream, 64 * ITER);

    delete[] keyVec;
    delete[] frameVec;
    delete[] ksBits;
    delete[] allKeystream;
    return 0;
}
