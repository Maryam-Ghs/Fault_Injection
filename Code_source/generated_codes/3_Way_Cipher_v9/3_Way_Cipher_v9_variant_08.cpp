#include <iostream>
#include <vector>
#include <climits>

/* LLM input variant 8: sparse-skewed */

int main() {
    // Version #9 - 3-Way Cipher implementation
    // Generate sparse, skewed test data
    std::vector<int> src(1000, 0);  // large vector mostly zeros

    // Insert a few extreme and typical values at scattered positions
    src[0] = INT_MAX;          // maximum int at start
    src[5] = 12345;            // typical positive near start
    src[10] = -54321;          // typical negative near start
    src[500] = INT_MIN;        // minimum int in the middle
    src[505] = 777777;         // another positive outlier
    src[510] = -999999;        // another negative outlier
    src[995] = 42;             // small positive near end
    src[999] = 0;              // explicit zero at end (already zero)

    // 3 independent keys for the cipher
    int keyA = 0x5A5A5A5A;   // pattern key
    int keyB = 0x7F7F7F7F;   // additive key
    int keyC = 0xFFFFFFFF;  // final xor key (all bits set)

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
    std::cout << "Original data (sparse representation, showing non-zero entries):\n";
    for (size_t i = 0; i < src.size(); ++i) {
        if (src[i] != 0) {
            std::cout << "index " << i << ": " << src[i] << "\n";
        }
    }
    std::cout << "\nEncrypted data (first 20 values):\n";
    for (size_t i = 0; i < std::min(enc.size(), static_cast<size_t>(20)); ++i) {
        std::cout << enc[i] << " ";
    }
    std::cout << "\n\nDecrypted data (first 20 values):\n";
    for (size_t i = 0; i < std::min(dec.size(), static_cast<size_t>(20)); ++i) {
        std::cout << dec[i] << " ";
    }
    std::cout << "\n\nEmpty vector test - encrypted size: " << encEmpty.size()
              << ", decrypted size: " << decEmpty.size() << "\n";

    return 0;
}
