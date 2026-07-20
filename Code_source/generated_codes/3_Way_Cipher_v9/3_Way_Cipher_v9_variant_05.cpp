#include <iostream>
#include <vector>
#include <climits>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // Version #9 - 3-Way Cipher implementation
    // Generate test data with heavy duplicate values
    std::vector<int> src;
    // Repeating pattern of values
    for (int i = 0; i < 3; ++i) {
        src.push_back(0);                     // zero repeated
        src.push_back(INT_MAX);              // maximum int repeated
        src.push_back(INT_MIN);              // minimum int repeated
        src.push_back(12345);                // typical positive repeated
        src.push_back(-54321);               // typical negative repeated
    }
    // also test an empty vector later

    // 3 independent keys for the cipher (duplicate-heavy)
    int keyA = 0x5A5A5A5A;   // pattern key
    int keyB = 0x5A5A5A5A;   // duplicate of keyA to create ties
    int keyC = 0x5A5A5A5A;   // duplicate of keyA and keyB

    // Helper lambda: encrypt a vector using the 3‑way scheme
    auto encrypt = [&](const std::vector<int>& plain) -> std::vector<int> {
        std::vector<int> cipher;
        size_t pos = 0;
        // use a do‑while loop to illustrate a different loop structure
        if (!plain.empty()) {
            do {
                int step1 = plain[pos] ^ keyA;          // first xor
                int step2 = step1 + keyB;               // then add
                int step3 = step2 ^ keyC;               // final xor
                cipher.push_back(step3);
                ++pos;
            } while (pos < plain.size());
        }
        return cipher;
    };

    // Helper lambda: decrypt the vector (reverse operations)
    auto decrypt = [&](const std::vector<int>& cipher) -> std::vector<int> {
        std::vector<int> recovered;
        size_t idx = 0;
        // use a while loop this time
        while (idx < cipher.size()) {
            int rev1 = cipher[idx] ^ keyC;              // undo final xor
            int rev2 = rev1 - keyB;                     // undo addition
            int rev3 = rev2 ^ keyA;                     // undo first xor
            recovered.push_back(rev3);
            ++idx;
        }
        return recovered;
    };

    // Perform encryption / decryption on the prepared data
    std::vector<int> enc = encrypt(src);
    std::vector<int> dec = decrypt(enc);

    // Also test an empty vector to cover edge case
    std::vector<int> emptyVec;
    std::vector<int> encEmpty = encrypt(emptyVec);
    std::vector<int> decEmpty = decrypt(encEmpty);

    // Output the results
    std::cout << "Original data:\n";
    for (int v : src) std::cout << v << " ";
    std::cout << "\n\nEncrypted data:\n";
    for (int v : enc) std::cout << v << " ";
    std::cout << "\n\nDecrypted data:\n";
    for (int v : dec) std::cout << v << " ";
    std::cout << "\n\nEmpty vector test - encrypted size: " << encEmpty.size()
              << ", decrypted size: " << decEmpty.size() << "\n";

    return 0;
}
