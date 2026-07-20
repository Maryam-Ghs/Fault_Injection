#include <iostream>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

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
    // 1. Prepare sparse, skewed predefined arrays (stack allocation)
    // -----------------------------------------------------------------
    // Plaintext block: mostly zeros, with a few scattered non‑zero values
    int plainBlock[16] = {
        72, 0, 0, 0, 0, 0, 0, 33,
        0, 0, 0, 0, 0, 101, 0, 0
    };
    // Keys: larger than typical, but sparsely populated
    int keyAlpha[10] = { 0, 0, 5, 0, 0, 0, 0, 7, 0, 0 };
    int keyBeta[7]   = { 0, 3, 0, 0, 0, 0, 9 };
    int keyGamma[5]  = { 0, 0, 2, 0, 0 };

    // Buffers for ciphertext and recovered plaintext
    int cipherBlock[16];
    int recovered[16];

    // -----------------------------------------------------------------
    // 2. Create cipher object and run encryption
    // -----------------------------------------------------------------
    ThreeWayCipher cipher;
    cipher.encryptBlock(plainBlock, cipherBlock,
                        keyAlpha, keyBeta, keyGamma,
                        16,                // block size
                        10, 7, 5);         // key sizes

    // -----------------------------------------------------------------
    // 3. Output results
    // -----------------------------------------------------------------
    std::cout << "Plaintext (decimal): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << plainBlock[i] << ' ';
    }
    std::cout << "\nPlaintext (hex)    : ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (plainBlock[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n\nCiphertext (decimal): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << cipherBlock[i] << ' ';
    }
    std::cout << "\nCiphertext (hex)    : ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipherBlock[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n";

    // -----------------------------------------------------------------
    // 4. Decrypt and verify
    // -----------------------------------------------------------------
    cipher.decryptBlock(cipherBlock, recovered,
                        keyAlpha, keyBeta, keyGamma,
                        16, 10, 7, 5);

    std::cout << "\nRecovered (decimal): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << recovered[i] << ' ';
    }
    std::cout << "\nRecovered (hex)    : ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (recovered[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n";

    // Simple verification flag
    int ok = 1;
    int idx = 0;
    while (idx < 16) {
        ok = ok * (plainBlock[idx] == recovered[idx]);
        idx = idx + 1;
    }
    std::cout << "\nVerification: " << (ok ? "SUCCESS" : "FAILURE") << "\n";

    return 0;
}
