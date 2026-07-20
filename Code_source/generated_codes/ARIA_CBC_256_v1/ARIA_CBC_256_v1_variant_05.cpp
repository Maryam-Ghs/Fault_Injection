#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */
int main() {
    /* -------------------------------------------------------------
       ARIA‑CBC‑256 implementation
       All data are stored as signed int (0‑255) – no unsigned types.
       Everything (tables, helpers, key schedule, encryption) lives
       inside main() as required.
       ------------------------------------------------------------- */

    /* ---------- 1.  S‑boxes (duplicate‑heavy) ---------- */
    std::vector<int> S1(256);
    // Fill with a small repeating pattern to create many duplicates
    int patternS1[4] = {0x00, 0xFF, 0xAA, 0x55};
    for (int i = 0; i < 256; ++i) S1[i] = patternS1[i % 4];

    std::vector<int> S2(256);
    // Use the same pattern for S2 to increase duplication
    int patternS2[4] = {0x00, 0xFF, 0xAA, 0x55};
    for (int i = 0; i < 256; ++i) S2[i] = patternS2[i % 4];
    // Note: The real S2 is the inverse of S1; for brevity we keep a dummy table.

    /* ---------- 2.  Round constants (C1‑C6) ---------- */
    // All round constants are set to the same value to maximize duplicates
    std::vector<int> C1(16, 0x5c);
    std::vector<int> C2(16, 0x5c);
    std::vector<int> C3(16, 0x5c);
    std::vector<int> C4(16, 0x5c);
    std::vector<int> C5(16, 0x5c);
    std::vector<int> C6(16, 0x5c);

    /* ---------- 3.  Helper lambdas ------------------------------------- */
    auto xorBlock = [&](std::vector<int>& a, const std::vector<int>& b) {
        for (int i = 0; i < 16; ++i) a[i] = (a[i] ^ b[i]) & 0xFF;
    };

    auto subBytes = [&](std::vector<int>& block, bool useS1) {
        const std::vector<int>& table = useS1 ? S1 : S2;
        for (int i = 0; i < 16; ++i) block[i] = table[block[i] & 0xFF];
    };

    auto diffusion = [&](std::vector<int>& block) {
        std::vector<int> t(16);
        t[0] = block[0] ^ block[3] ^ block[5] ^ block[7] ^ block[12];
        t[1] = block[0] ^ block[1] ^ block[4] ^ block[6] ^ block[13];
        t[2] = block[1] ^ block[2] ^ block[5] ^ block[7] ^ block[14];
        t[3] = block[2] ^ block[3] ^ block[6] ^ block[8] ^ block[15];
        t[4] = block[0] ^ block[4] ^ block[5] ^ block[8] ^ block[10];
        t[5] = block[1] ^ block[5] ^ block[6] ^ block[9] ^ block[11];
        t[6] = block[2] ^ block[6] ^ block[7] ^ block[10] ^ block[12];
        t[7] = block[3] ^ block[7] ^ block[8] ^ block[11] ^ block[13];
        t[8] = block[0] ^ block[8] ^ block[9] ^ block[12] ^ block[14];
        t[9] = block[1] ^ block[9] ^ block[10] ^ block[13] ^ block[15];
        t[10]= block[2] ^ block[10]^ block[11]^ block[14]^ block[0];
        t[11]= block[3] ^ block[11]^ block[12]^ block[15]^ block[1];
        t[12]= block[4] ^ block[12]^ block[13]^ block[0] ^ block[2];
        t[13]= block[5] ^ block[13]^ block[14]^ block[1] ^ block[3];
        t[14]= block[6] ^ block[14]^ block[15]^ block[2] ^ block[4];
        t[15]= block[7] ^ block[15]^ block[0] ^ block[3] ^ block[5];
        for (int i = 0; i < 16; ++i) block[i] = t[i] & 0xFF;
    };

    auto rotLeft = [&](std::vector<int> v, int n) {
        std::vector<int> r(16);
        for (int i = 0; i < 16; ++i) r[i] = v[(i + n) % 16];
        return r;
    };

    /* ---------- 4.  Key schedule for 256‑bit key ---------------------- */
    std::vector<int> masterKey(32, 0xAA); // duplicate‑heavy key (all bytes the same)

    // Split master key into two 16‑byte halves
    std::vector<int> KL(masterKey.begin(), masterKey.begin() + 16);
    std::vector<int> KR(masterKey.begin() + 16, masterKey.end());

    // Generate W0 … W3 (intermediate keys)
    std::vector<int> W0(16), W1(16), W2(16), W3(16);
    // W0 = KL
    W0 = KL;

    // W1 = F(KR ⊕ C1) ⊕ KL
    std::vector<int> temp = KR;
    xorBlock(temp, C1);
    subBytes(temp, true);   // S1
    diffusion(temp);
    subBytes(temp, false);  // S2
    xorBlock(temp, KL);
    W1 = temp;

    // W2 = F(W1 ⊕ C2) ⊕ KR
    temp = W1;
    xorBlock(temp, C2);
    subBytes(temp, true);
    diffusion(temp);
    subBytes(temp, false);
    xorBlock(temp, KR);
    W2 = temp;

    // W3 = F(W2 ⊕ C3) ⊕ W0
    temp = W2;
    xorBlock(temp, C3);
    subBytes(temp, true);
    diffusion(temp);
    subBytes(temp, false);
    xorBlock(temp, W0);
    W3 = temp;

    // Assemble round keys RK0 … RK12 (13 keys). For 256‑bit we need 13 subkeys.
    // The pattern is: RK0 = KL, RK1 = W0 ⊕ C1, RK2 = W1 ⊕ C2, …
    std::vector<std::vector<int>> roundKey(13, std::vector<int>(16));
    roundKey[0] = KL;
    roundKey[1] = W0; xorBlock(roundKey[1], C1);
    roundKey[2] = W1; xorBlock(roundKey[2], C2);
    roundKey[3] = W2; xorBlock(roundKey[3], C3);
    roundKey[4] = W3; xorBlock(roundKey[4], C4);
    roundKey[5] = W0; xorBlock(roundKey[5], C5);
    roundKey[6] = W1; xorBlock(roundKey[6], C6);
    roundKey[7] = W2; xorBlock(roundKey[7], C1);
    roundKey[8] = W3; xorBlock(roundKey[8], C2);
    roundKey[9] = W0; xorBlock(roundKey[9], C3);
    roundKey[10]= W1; xorBlock(roundKey[10],C4);
    roundKey[11]= W2; xorBlock(roundKey[11],C5);
    roundKey[12]= W3; xorBlock(roundKey[12],C6);

    /* ---------- 5.  Block encryption (one 16‑byte block) -------------- */
    auto encryptBlock = [&](std::vector<int> plain) {
        // Initial key addition
        xorBlock(plain, roundKey[0]);

        // 11 full rounds (for 256‑bit: 12 rounds total, last round is slightly different)
        for (int r = 1; r <= 11; ++r) {
            // odd round: S1 → diffusion → S2
            // even round: S2 → diffusion → S1
            if (r % 2 == 1) {
                subBytes(plain, true);
                diffusion(plain);
                subBytes(plain, false);
            } else {
                subBytes(plain, false);
                diffusion(plain);
                subBytes(plain, true);
            }
            xorBlock(plain, roundKey[r]);
        }

        // Final round (round 12) – only substitution and key addition
        subBytes(plain, true);
        xorBlock(plain, roundKey[12]);
        return plain;
    };

    /* ---------- 6.  CBC mode – deterministic test vectors ------------ */
    std::vector<int> IV(16, 0x00); // duplicate‑heavy IV (all zeros)

    // Plaintext: two blocks (32 bytes) – duplicate‑heavy pattern
    std::vector<int> plaintext(32, 0x33); // all bytes the same

    // CBC encryption
    std::vector<int> previous = IV;
    std::vector<int> ciphertext; ciphertext.reserve(32);

    for (int blk = 0; blk < 2; ++blk) {
        std::vector<int> block(16);
        for (int i = 0; i < 16; ++i) block[i] = plaintext[blk * 16 + i];
        xorBlock(block, previous);                 // XOR with IV / previous cipher
        block = encryptBlock(block);                // Encrypt
        for (int i = 0; i < 16; ++i) ciphertext.push_back(block[i]);
        previous = block;                           // Update chaining value
    }

    /* ---------- 7.  Print results ------------------------------------ */
    std::cout << "Key (256‑bit) : ";
    for (int i = 0; i < 32; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << masterKey[i];
    std::cout << std::dec << "\n";

    std::cout << "IV            : ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << IV[i];
    std::cout << std::dec << "\n";

    std::cout << "Plaintext     : ";
    for (int i = 0; i < 32; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << plaintext[i];
    std::cout << std::dec << "\n";

    std::cout << "Ciphertext(CBC‑256) : ";
    for (size_t i = 0; i < ciphertext.size(); ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << ciphertext[i];
    std::cout << std::dec << "\n";

    return 0;
}
