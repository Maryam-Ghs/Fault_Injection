#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */
int main() {
    // ------------------------------------------------------------
    // ARIA-CFB (Version #6) – deterministic pseudo‑random test data.
    // ------------------------------------------------------------

    int BLOCK = 16;
    int KEYLEN = 16;
    int ROUNDS = 3;

    // ----------------------------------------------------------------
    // 1️⃣  Deterministic pseudo‑random keys (seed = 42)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > keyBank;
    int seed = 42;
    int i = 0;
    while (i < 3) {               // three keys
        std::vector<int> key(KEYLEN);
        int j = 0;
        while (j < KEYLEN) {
            // simple linear congruential generator step
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            key[j] = (seed >> 16) & 0xFF;
            j = j + 1;
        }
        keyBank.push_back(key);
        i = i + 1;
    }

    // ----------------------------------------------------------------
    // 2️⃣  Deterministic pseudo‑random plaintexts (different seeds)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > plainBank;
    int baseSeed = 1001;
    i = 0;
    while (i < 4) {               // four plaintexts of size 64
        std::vector<int> plain(64);
        int local = baseSeed + i * 7;
        int j = 0;
        while (j < 64) {
            local = (local * 1664525 + 1013904223) & 0xFFFFFFFF;
            plain[j] = (local >> 24) & 0xFF;
            j = j + 1;
        }
        plainBank.push_back(plain);
        i = i + 1;
    }

    // ----------------------------------------------------------------
    // 3️⃣  Fixed IV
    // ----------------------------------------------------------------
    std::vector<int> ivFixed(BLOCK);
    i = 0;
    while (i < BLOCK) {
        ivFixed[i] = (i * 9) & 0xFF;
        i = i + 1;
    }

    // ----------------------------------------------------------------
    // 4️⃣  Iterate over keys and plaintexts
    // ----------------------------------------------------------------
    int keyIdx = 0;
    while (keyIdx < (int)keyBank.size()) {
        std::vector<int> curKey = keyBank[keyIdx];

        // round keys
        std::vector< std::vector<int> > roundKeySet;
        int r = 0;
        while (r < ROUNDS) {
            std::vector<int> rk(KEYLEN);
            int j = 0;
            while (j < KEYLEN) {
                rk[j] = curKey[j] ^ (r * 0x1F);
                j = j + 1;
            }
            roundKeySet.push_back(rk);
            r = r + 1;
        }

        int ptIdx = 0;
        while (ptIdx < (int)plainBank.size()) {
            std::vector<int> curPlain = plainBank[ptIdx];
            std::vector<int> feedback = ivFixed;
            std::vector<int> cipherText;

            int pos = 0;
            while (pos < (int)curPlain.size()) {
                std::vector<int> encBlock = feedback;
                int round = 0;
                while (round < ROUNDS) {
                    std::vector<int> curRoundKey = roundKeySet[round];
                    int byte = 0;
                    while (byte < BLOCK) {
                        encBlock[byte] ^= curRoundKey[byte];
                        byte = byte + 1;
                    }
                    round = round + 1;
                }

                int remain = (int)curPlain.size() - pos;
                int chunk = BLOCK;
                if (remain < BLOCK) chunk = remain;

                int k = 0;
                while (k < chunk) {
                    int ctByte = curPlain[pos + k] ^ encBlock[k];
                    cipherText.push_back(ctByte);

                    int shift = 0;
                    while (shift < BLOCK - 1) {
                        feedback[shift] = feedback[shift + 1];
                        shift = shift + 1;
                    }
                    feedback[BLOCK - 1] = ctByte;
                    k = k + 1;
                }
                pos = pos + chunk;
            }

            std::cout << "KeyIdx=" << keyIdx
                      << " PlainIdx=" << ptIdx
                      << " CipherHex=";
            int out = 0;
            while (out < (int)cipherText.size()) {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << (cipherText[out] & 0xFF);
                out = out + 1;
            }
            std::cout << std::dec << std::endl;

            ptIdx = ptIdx + 1;
        }

        keyIdx = keyIdx + 1;
    }

    return 0;
}
