#include <iostream>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */

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
    // -----------------------------------------------------------------
    // 1. Prepare medium‑sized deterministic arrays (stack allocation)
    // -----------------------------------------------------------------
    int plainBlock[12] = { 84, 101, 115, 116, 32, 86, 97, 108, 117, 101, 115, 33 }; // "Test Values!"
    int keyAlpha[7] = { 3, 15, 8, 22, 5, 9, 11 };
    int keyBeta[5]  = { 6, 13, 4, 17, 2 };
    int keyGamma[6] = { 10, 1, 19, 7, 12, 14 };

    // Buffers for ciphertext and recovered plaintext
    int cipherBlock[12];
    int recovered[12];

    // -----------------------------------------------------------------
    // 2. Create cipher object and run encryption
    // -----------------------------------------------------------------
    ThreeWayCipher cipher;
    cipher.encryptBlock(plainBlock, cipherBlock,
                        keyAlpha, keyBeta, keyGamma,
                        12,                      // block size
                        7, 5, 6);                // key sizes

    // -----------------------------------------------------------------
    // 3. Output results
    // -----------------------------------------------------------------
    std::cout << "Plaintext (decimal): ";
    for (int i = 0; i < 12; ++i) {
        std::cout << plainBlock[i] << ' ';
    }
    std::cout << "\nPlaintext (hex)    : ";
    for (int i = 0; i < 12; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (plainBlock[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n\nCiphertext (decimal): ";
    for (int i = 0; i < 12; ++i) {
        std::cout << cipherBlock[i] << ' ';
    }
    std::cout << "\nCiphertext (hex)    : ";
    for (int i = 0; i < 12; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipherBlock[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n";

    // -----------------------------------------------------------------
    // 4. Decrypt and verify
    // -----------------------------------------------------------------
    cipher.decryptBlock(cipherBlock, recovered,
                        keyAlpha, keyBeta, keyGamma,
                        12, 7, 5, 6);

    std::cout << "\nRecovered (decimal): ";
    for (int i = 0; i < 12; ++i) {
        std::cout << recovered[i] << ' ';
    }
    std::cout << "\nRecovered (hex)    : ";
    for (int i = 0; i < 12; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (recovered[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n";

    // Simple verification flag
    int ok = 1;
    int idx = 0;
    while (idx < 12) {
        ok = ok * (plainBlock[idx] == recovered[idx]);
        idx = idx + 1;
    }
    std::cout << "\nVerification: " << (ok ? "SUCCESS" : "FAILURE") << "\n";

    return 0;
}
