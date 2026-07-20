#include <iostream>

/* LLM input variant 10: large-safe-stress */

int main()
{
    // -------------------------------------------------------
    // 1. Parameters and pseudo‑random generator (LCG)
    // -------------------------------------------------------
    int blockCount = 2000;                    // number of 64‑bit blocks (large but safe)
    int seed = 987654321;                     // LCG seed

    // -------------------------------------------------------
    // 2. Allocate all data on the heap (plain, cipher, keystream)
    // -------------------------------------------------------
    int* plain   = new int[blockCount];
    int* cipher  = new int[blockCount];
    int* stream  = new int[blockCount];

    // -------------------------------------------------------
    // 3. Fill the plaintext with “random” byte values (0‑255)
    // -------------------------------------------------------
    for (int i = 0; i < blockCount; ++i)
    {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        plain[i] = seed & 0xff;
    }

    // -------------------------------------------------------
    // 4. Keys (three independent 64‑bit keys, stored as two 32‑bit halves)
    // -------------------------------------------------------
    int key1Hi = 0x13345779, key1Lo = 0x9ABCDEF0;
    int key2Hi = 0x2468ACE0, key2Lo = 0x0FEDCBA9;
    int key3Hi = 0x1A2B3C4D, key3Lo = 0x87654321;

    // -------------------------------------------------------
    // 5. Create 16 round sub‑keys for each main key (simple rotations)
    // -------------------------------------------------------
    int subK1[16], subK2[16], subK3[16];
    for (int r = 0; r < 16; ++r)
    {
        int rot = r & 31;                                     // rotation count
        subK1[r] = ((key1Hi << rot) | (key1Hi >> (32-rot))) ^ ((key1Lo << rot) | (key1Lo >> (32-rot)));
        subK2[r] = ((key2Hi << rot) | (key2Hi >> (32-rot))) ^ ((key2Lo << rot) | (key2Lo >> (32-rot)));
        subK3[r] = ((key3Hi << rot) | (key3Hi >> (32-rot))) ^ ((key3Lo << rot) | (key3Lo >> (32-rot)));
    }

    // -------------------------------------------------------
    // 6. Feistel round function (expanded, multi‑step)
    // -------------------------------------------------------
    auto fFunc = [](int half, int sub) -> int
    {
        int step1 = (half + sub) & 0xffffffff;
        int step2 = ((step1 << 5) | (step1 >> 27)) & 0xffffffff;
        int step3 = ((step2 ^ 0x5A5A5A5A) + (step2 >> 3)) & 0xffffffff;
        int step4 = ((step3 << 2) | (step3 >> 30)) & 0xffffffff;
        return step4;
    };

    // -------------------------------------------------------
    // 7. Triple‑DES encryption of a 64‑bit block (left/right halves)
    // -------------------------------------------------------
    auto tripleDesEnc = [&](int L, int R) -> std::pair<int,int>
    {
        // ---- first DES (encrypt with key1) ----
        int L1 = L, R1 = R;
        for (int r = 0; r < 16; ++r)
        {
            int tmp = R1;
            R1 = L1 ^ fFunc(R1, subK1[r]);
            L1 = tmp;
        }
        // swap after last round
        int tmp = L1; L1 = R1; R1 = tmp;

        // ---- second DES (decrypt with key2) ----
        int L2 = L1, R2 = R1;
        for (int r = 15; r >= 0; --r)
        {
            int tmp2 = L2;
            L2 = R2 ^ fFunc(L2, subK2[r]);
            R2 = tmp2;
        }
        // swap after last round
        tmp = L2; L2 = R2; R2 = tmp;

        // ---- third DES (encrypt with key3) ----
        int L3 = L2, R3 = R2;
        for (int r = 0; r < 16; ++r)
        {
            int tmp3 = R3;
            R3 = L3 ^ fFunc(R3, subK3[r]);
            L3 = tmp3;
        }
        // final swap
        tmp = L3; L3 = R3; R3 = tmp;

        return std::make_pair(L3, R3);
    };

    // -------------------------------------------------------
    // 8. Initialise OFB vector (IV) – also on the heap for symmetry
    // -------------------------------------------------------
    int* iv = new int[2];
    iv[0] = 0x01234567;          // high 32‑bits
    iv[1] = 0x89ABCDEF;          // low  32‑bits

    // -------------------------------------------------------
    // 9. OFB processing loop (feedback → keystream → XOR)
    // -------------------------------------------------------
    int fbHi = iv[0];
    int fbLo = iv[1];
    for (int i = 0; i < blockCount; ++i)
    {
        // generate next keystream block
        std::pair<int,int> ks = tripleDesEnc(fbHi, fbLo);
        int ksWord = ks.first ^ ks.second;          // compress to a single 32‑bit word
        stream[i] = ksWord;

        // encrypt plaintext block
        cipher[i] = plain[i] ^ ksWord;

        // feed the keystream back into OFB
        fbHi = ks.first;
        fbLo = ks.second;
    }

    // -------------------------------------------------------
    // 10. Output (hexadecimal) – plain and cipher
    // -------------------------------------------------------
    std::cout << "Plaintext : ";
    for (int i = 0; i < blockCount; ++i)
        std::cout << std::hex << (plain[i] & 0xff) << ' ';
    std::cout << "\nCiphertext: ";
    for (int i = 0; i < blockCount; ++i)
        std::cout << std::hex << (cipher[i] & 0xffffffff) << ' ';
    std::cout << std::endl;

    // -------------------------------------------------------
    // 11. Clean up heap memory
    // -------------------------------------------------------
    delete[] plain;
    delete[] cipher;
    delete[] stream;
    delete[] iv;

    return 0;
}
