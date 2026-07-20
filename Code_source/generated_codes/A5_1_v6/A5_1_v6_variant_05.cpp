/ * LLM input variant 5: duplicate-heavy */
#include <iostream>
#include <vector>

class A5_1 {
public:
    // Constructor creates empty registers
    A5_1() {
        reg1.assign(19, 0);
        reg2.assign(22, 0);
        reg3.assign(23, 0);
    }

    // Load a 64‑bit key and a 22‑bit frame number
    void load(const std::vector<int>& keyBits,
              const std::vector<int>& frameBits) {
        int i, bit;

        // 1) Fill all registers with the key bits (XOR into the current content)
        for (i = 0; i < 64; ++i) {
            bit = keyBits[i];
            xorInto(reg1, i % 19, bit);
            xorInto(reg2, i % 22, bit);
            xorInto(reg3, i % 23, bit);
        }

        // 2) Mix in the frame number (also XOR)
        for (i = 0; i < 22; ++i) {
            bit = frameBits[i];
            xorInto(reg1, i % 19, bit);
            xorInto(reg2, i % 22, bit);
            xorInto(reg3, i % 23, bit);
        }

        // 3) Run 100 warm‑up cycles (clock with majority rule, no output)
        for (i = 0; i < 100; ++i) {
            clockRegisters();
        }
    }

    // Generate a single keystream bit
    int generateBit() {
        int out1 = reg1[18];
        int out2 = reg2[21];
        int out3 = reg3[22];
        int keystream = out1 ^ out2 ^ out3;

        clockRegisters();               // advance the registers
        return keystream;
    }

private:
    std::vector<int> reg1;   // 19 bits
    std::vector<int> reg2;   // 22 bits
    std::vector<int> reg3;   // 23 bits

    // XOR a bit into a given position of a register
    void xorInto(std::vector<int>& r, int pos, int val) {
        r[pos] = r[pos] ^ val;
    }

    // Compute the majority of three bits
    int majority(int a, int b, int c) {
        int sum = a + b + c;
        return (sum >= 2) ? 1 : 0;
    }

    // One clock step respecting the majority rule
    void clockRegisters() {
        int clk1 = reg1[8];
        int clk2 = reg2[10];
        int clk3 = reg3[10];
        int maj  = majority(clk1, clk2, clk3);

        // Temporary variables hold the new feedback bits
        int fb1 = (reg1[13] ^ reg1[16] ^ reg1[17] ^ reg1[18]) & 1;
        int fb2 = (reg2[20] ^ reg2[21]) & 1;
        int fb3 = (reg3[7]  ^ reg3[20] ^ reg3[21] ^ reg3[22]) & 1;

        // Shift registers that agree with the majority
        if (clk1 == maj) shift(reg1, fb1);
        if (clk2 == maj) shift(reg2, fb2);
        if (clk3 == maj) shift(reg3, fb3);
    }

    // Shift a register right, inserting newBit at position 0
    void shift(std::vector<int>& r, int newBit) {
        for (int i = static_cast<int>(r.size()) - 1; i > 0; --i) {
            r[i] = r[i - 1];
        }
        r[0] = newBit & 1;
    }
};

int main() {
    // 1) Prepare a deterministic set of duplicate‑heavy keys and frames
    const int batchSize = 10;                     // number of independent encryptions
    std::vector< std::vector<int> > keyPool;
    std::vector< std::vector<int> > framePool;

    // Create a repeating pattern: 0,0,1,1 for keys (64 bits)
    std::vector<int> baseKey(64);
    for (int i = 0; i < 64; i += 4) {
        baseKey[i] = 0;
        baseKey[i + 1] = 0;
        baseKey[i + 2] = 1;
        baseKey[i + 3] = 1;
    }

    // Create a repeating pattern: 1,1,0,0 for frames (22 bits)
    std::vector<int> baseFrame(22);
    for (int i = 0; i < 22; i += 4) {
        baseFrame[i] = 1;
        baseFrame[i + 1] = 1;
        if (i + 2 < 22) baseFrame[i + 2] = 0;
        if (i + 3 < 22) baseFrame[i + 3] = 0;
    }

    // Fill pools with identical duplicate entries
    for (int i = 0; i < batchSize; ++i) {
        keyPool.push_back(baseKey);
        framePool.push_back(baseFrame);
    }

    // 2) Run A5/1 for each (key,frame) pair and collect 20 keystream bits
    for (int i = 0; i < batchSize; ++i) {
        A5_1 cipher;
        cipher.load(keyPool[i], framePool[i]);

        std::cout << "Keystream " << i << ": ";
        for (int j = 0; j < 20; ++j) {
            int bit = cipher.generateBit();
            std::cout << bit;
        }
        std::cout << std::endl;
    }

    return 0;
}
