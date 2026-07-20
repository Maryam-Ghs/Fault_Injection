#include <iostream>
#include <iomanip>

/* LLM input variant 2: small-diverse */

class ThreeWayCipher {
public:
    // Encrypt a block using three key blocks
    void encryptBlock(const int* plain, int* crypt,
                      const int* keyA, const int* keyB, const int* keyC,
                      int blockSize,
                      int sizeA, int sizeB, int sizeC) {
        int idx = 0;
        while (idx < blockSize) {
            // Step‑1: add keyA
            int step1 = plain[idx] + keyA[idx % sizeA];
            // Step‑2: XOR with keyB
            int step2 = step1 ^ keyB[idx % sizeB];
            // Step‑3: subtract keyC then add keyA again (reordered arithmetic)
            int step3 = (step2 - keyC[idx % sizeC]) + keyA[idx % sizeA];
            crypt[idx] = step3;
            idx = idx + 1;               // explicit increment (while‑loop style)
        }
    }

    // Decrypt a block using the same three key blocks
    void decryptBlock(const int* crypt, int* plain,
                      const int* keyA, const int* keyB, const int* keyC,
                      int blockSize,
                      int sizeA, int sizeB, int sizeC) {
        int pos = 0;
        while (pos < blockSize) {
            // Reverse Step‑3
            int rev3 = crypt[pos] - keyA[pos % sizeA] + keyC[pos % sizeC];
            // Reverse Step‑2
            int rev2 = rev3 ^ keyB[pos % sizeB];
            // Reverse Step‑1
            int rev1 = rev2 - keyA[pos % sizeA];
            plain[pos] = rev1;
            pos = pos + 1;               // explicit increment
        }
    }
};

int main() {
    const int blockSize = 7;

    // -----------------------------------------------------------------
    // 1. Prepare small predefined arrays (stack allocation)
    // -----------------------------------------------------------------
    int plainBlock[blockSize] = { 84, 101, 115, 116, 105, 110, 103 };   // "Testing"
    int keyAlpha[6] = { 3, 11, 7, 2, 9, 5 };
    int keyBeta[5]  = { 4, 6, 8, 10, 12 };
    int keyGamma[4] = { 1, 13, 15, 2 };

    // Buffers for ciphertext and recovered plaintext
    int cipherBlock[blockSize];
    int recovered[blockSize];

    // -----------------------------------------------------------------
    // 2. Create cipher object and run encryption
    // -----------------------------------------------------------------
    ThreeWayCipher cipher;
    cipher.encryptBlock(plainBlock, cipherBlock,
                        keyAlpha, keyBeta, keyGamma,
                        blockSize,
                        6, 5, 4);

    // -----------------------------------------------------------------
    // 3. Output results
    // -----------------------------------------------------------------
    std::cout << "Plaintext (decimal): ";
    for (int i = 0; i < blockSize; ++i) {
        std::cout << plainBlock[i] << ' ';
    }
    std::cout << "\nPlaintext (hex)    : ";
    for (int i = 0; i < blockSize; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (plainBlock[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n\nCiphertext (decimal): ";
    for (int i = 0; i < blockSize; ++i) {
        std::cout << cipherBlock[i] << ' ';
    }
    std::cout << "\nCiphertext (hex)    : ";
    for (int i = 0; i < blockSize; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipherBlock[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n";

    // -----------------------------------------------------------------
    // 4. Decrypt and verify
    // -----------------------------------------------------------------
    cipher.decryptBlock(cipherBlock, recovered,
                        keyAlpha, keyBeta, keyGamma,
                        blockSize, 6, 5, 4);

    std::cout << "\nRecovered (decimal): ";
    for (int i = 0; i < blockSize; ++i) {
        std::cout << recovered[i] << ' ';
    }
    std::cout << "\nRecovered (hex)    : ";
    for (int i = 0; i < blockSize; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (recovered[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n";

    // Simple verification flag
    int ok = 1;
    int idx = 0;
    while (idx < blockSize) {
        ok = ok * (plainBlock[idx] == recovered[idx]);
        idx = idx + 1;
    }
    std::cout << "\nVerification: " << (ok ? "SUCCESS" : "FAILURE") << "\n";

    return 0;
}
