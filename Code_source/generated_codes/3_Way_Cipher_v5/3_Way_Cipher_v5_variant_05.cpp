/* LLM input variant 5: duplicate-heavy */
// 3‑Way Cipher – version #5
// ------------------------------------------------------------
// Implements a simple three‑step reversible cipher on large
// random integer arrays.  All code resides inside main() and
// uses only int and float types, std::vector for storage, and
// modular lambdas for clarity.

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

int main()
{
    // --------------------------------------------------------
    // 1.  Helper lambdas (function‑based modular design)
    // --------------------------------------------------------
    // Generate a vector filled with a deterministic repeating pattern
    auto make_random_vec = [&](int length) -> std::vector<int>
    {
        std::vector<int> result;
        result.reserve(length);
        int idx = 0;
        while (idx < length)
        {
            // Pattern: 0, 85, 170, 255 repeating -> many duplicates
            int value = (idx % 4) * 85;
            result.push_back(value);
            ++idx;
        }
        return result;
    };

    // Rotate an 8‑bit value left by r bits
    auto rotl8 = [&](int val, int r) -> int
    {
        int mask = 0xFF;
        int left  = (val << r) & mask;
        int right = (val >> (8 - r)) & mask;
        return (left | right) & mask;
    };

    // Rotate an 8‑bit value right by r bits
    auto rotr8 = [&](int val, int r) -> int
    {
        int mask = 0xFF;
        int right = (val >> r) & mask;
        int left  = (val << (8 - r)) & mask;
        return (right | left) & mask;
    };

    // One encryption pass (three distinct mathematical steps)
    auto encrypt_vec = [&](const std::vector<int>& src,
                           const std::vector<int>& key) -> std::vector<int>
    {
        std::vector<int> dst;
        dst.reserve(src.size());

        int i = 0;
        while (i < (int)src.size())
        {
            // step‑1 : addition with key, masked to 8 bits
            int step1 = (src[i] + key[i]) & 0xFF;

            // step‑2 : XOR with a shifted key, masked again
            int step2 = (step1 ^ ((key[i] << 1) & 0xFF)) & 0xFF;

            // step‑3 : rotate left by 2 bits
            int step3 = rotl8(step2, 2);

            dst.push_back(step3);
            ++i;
        }
        return dst;
    };

    // One decryption pass (inverse of the three steps above)
    auto decrypt_vec = [&](const std::vector<int>& src,
                           const std::vector<int>& key) -> std::vector<int>
    {
        std::vector<int> dst;
        dst.reserve(src.size());

        int i = 0;
        while (i < (int)src.size())
        {
            // inverse‑step‑3 : rotate right by 2 bits
            int inv3 = rotr8(src[i], 2);

            // inverse‑step‑2 : XOR with the same shifted key
            int inv2 = (inv3 ^ ((key[i] << 1) & 0xFF)) & 0xFF;

            // inverse‑step‑1 : subtract the key, keep within 0‑255
            int inv1 = (inv2 - key[i]) & 0xFF;
            if (inv1 < 0) inv1 += 256;          // ensure non‑negative

            dst.push_back(inv1);
            ++i;
        }
        return dst;
    };

    // Simple printer for the first N elements of a vector
    auto show_some = [&](const std::vector<int>& vec, int count, const char* tag)
    {
        std::cout << tag << " : ";
        int limit = count;
        if (limit > (int)vec.size()) limit = vec.size();
        for (int j = 0; j < limit; ++j)
        {
            std::cout << vec[j];
            if (j + 1 < limit) std::cout << ", ";
        }
        std::cout << std::endl;
    };

    // --------------------------------------------------------
    // 2.  Generate large deterministic data with many duplicates
    // --------------------------------------------------------
    // No need for random seed; data is deterministic
    const int ARRAY_LEN = 50000;                // large size
    std::vector<int> plain = make_random_vec(ARRAY_LEN);
    // Secret key: alternating 0 and 255 -> many duplicate keys
    std::vector<int> secret;
    secret.reserve(ARRAY_LEN);
    for (int i = 0; i < ARRAY_LEN; ++i)
        secret.push_back((i % 2) ? 255 : 0);

    // --------------------------------------------------------
    // 3.  Encrypt and then decrypt
    // --------------------------------------------------------
    std::vector<int> cipher = encrypt_vec(plain, secret);
    std::vector<int> recovered = decrypt_vec(cipher, secret);

    // --------------------------------------------------------
    // 4.  Display a tiny sample to verify correctness
    // --------------------------------------------------------
    const int SAMPLE = 10;
    show_some(plain,     SAMPLE, "Plain      ");
    show_some(cipher,    SAMPLE, "Cipher     ");
    show_some(recovered, SAMPLE, "Recovered  ");

    // Simple sanity check (optional output)
    bool ok = true;
    int idx = 0;
    while (idx < ARRAY_LEN)
    {
        if (plain[idx] != recovered[idx])
        {
            ok = false;
            break;
        }
        ++idx;
    }
    std::cout << "Verification: " << (ok ? "SUCCESS" : "FAILURE") << std::endl;

    return 0;
}
