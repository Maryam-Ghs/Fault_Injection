// A5/2 implementation – version #9
// Uses only int, std::vector, and class‑based design.
// Generates random internal seeds and prints a keystream.
/* LLM input variant 4: signed-extremes */

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

class A52Cipher {
public:
    // Constructor: initialise registers with supplied seed vectors
    A52Cipher(const std::vector<int>& seedX,
              const std::vector<int>& seedY,
              const std::vector<int>& seedZ) {
        // copy only the required number of bits
        for (int i = 0; i < 19; ++i) regX.push_back(seedX[i] & 1);
        for (int i = 0; i < 22; ++i) regY.push_back(seedY[i] & 1);
        for (int i = 0; i < 23; ++i) regZ.push_back(seedZ[i] & 1);
    }

    // Produce one keystream bit
    int produceBit() {
        int maj = majority(regX[8], regY[10], regZ[10]);

        // Clock registers whose control bit equals the majority
        if (regX[8] == maj) clockX();
        if (regY[10] == maj) clockY();
        // Z is always clocked in A5/2
        clockZ();

        // Output is XOR of the last bits of each register
        int out = regX[18] ^ regY[21] ^ regZ[22];
        return out;
    }

private:
    std::vector<int> regX; // 19‑bit LFSR
    std::vector<int> regY; // 22‑bit LFSR
    std::vector<int> regZ; // 23‑bit LFSR

    // Majority function for three bits
    int majority(int a, int b, int c) {
        int sum = a + b + c;
        return (sum >= 2) ? 1 : 0;
    }

    // Clock the X register (feedback taps 13, 16, 17, 18)
    void clockX() {
        int newBit = regX[13] ^ regX[16] ^ regX[17] ^ regX[18];
        shiftLeft(regX, newBit);
    }

    // Clock the Y register (feedback taps 20, 21)
    void clockY() {
        int newBit = regY[20] ^ regY[21];
        shiftLeft(regY, newBit);
    }

    // Clock the Z register (feedback taps 7, 20, 21, 22)
    void clockZ() {
        int newBit = regZ[7] ^ regZ[20] ^ regZ[21] ^ regZ[22];
        shiftLeft(regZ, newBit);
    }

    // Generic left‑shift with insertion of new bit at position 0
    void shiftLeft(std::vector<int>& reg, int newBit) {
        // shift all bits one position towards the MSB
        int idx = reg.size() - 1;
        while (idx > 0) {
            reg[idx] = reg[idx - 1];
            idx = idx - 1;
        }
        reg[0] = newBit & 1;
    }
};

// Helper: create a random vector of given length (values 0 or 1)
std::vector<int> randomBits(int length) {
    std::vector<int> vec;
    int i = 0;
    while (i < length) {
        int bit = rand() % 2;
        vec.push_back(bit);
        i = i + 1;
    }
    return vec;
}

int main() {
    // Deterministic seed vectors mixing negative, zero, and positive ints
    std::vector<int> seedX = {
        -2147483648, -123456789, -1, 0, 1,
        123456789, 2147483647, -2, 2, -3,
        3, -4, 4, -5, 5,
        -6, 6, -7, 7
    };
    std::vector<int> seedY = {
        -2147483648, -100000000, -2, 0, 2,
        100000000, 2147483647, -3, 3, -4,
        4, -5, 5, -6, 6,
        -7, 7, -8, 8, -9,
        9, -10
    };
    std::vector<int> seedZ = {
        -2147483648, -99999999, -3, 0, 3,
        99999999, 2147483647, -4, 4, -5,
        5, -6, 6, -7, 7,
        -8, 8, -9, 9, -10,
        10, -11, 11
    };

    // create cipher instance
    A52Cipher cipher(seedX, seedY, seedZ);

    // produce and print 64 keystream bits
    int produced = 0;
    while (produced < 64) {
        int bit = cipher.produceBit();
        std::cout << bit;
        produced = produced + 1;
        if (produced % 8 == 0) std::cout << ' ';
    }
    std::cout << std::endl;
    return 0;
}
