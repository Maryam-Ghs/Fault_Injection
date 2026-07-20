#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */
int main() {
    // ------------------------------------------------------------
    // ARIA-CFB (Version #6) – large but safe stress test.
    // ------------------------------------------------------------

    int BLOCK = 16;
    int KEYLEN = 16;
    int ROUNDS = 12;

    // ----------------------------------------------------------------
    // 1️⃣  Five distinct high‑entropy keys
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > keyBank;
    int seed = 987654321;
    for (int i = 0; i < 5; ++i) {
        std::vector<int> key(KEYLEN);
        int local = seed + i * 12345;
        for (int j = 0; j < KEYLEN; ++j) {
            local = (local * 22695477 + 1) & 0xFFFFFFFF;
            key[j] = (local >> 16) & 0xFF;
        }
        keyBank.push_back(key);
    }

    // ----------------------------------------------------------------
    // 2️⃣  Plaintexts: three large blocks (1024, 2048, 3072 bytes)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > plainBank;

    const int sizes[3] = {1024, 2048, 3072};
    for (int s = 0; s < 3; ++s) {
        std::vector<int> p(sizes[s]);
        int val = 0;
        for (int i = 0; i < sizes[s]; ++i) {
            val = (val + 37) & 0xFF;          // simple deterministic pattern
            p[i] = val;
        }
        plainBank.push_back(p);
    }

    // ----------------------------------------------------------------
    // 3️⃣  Fixed IV
    // ----------------------------------------------------------------
    std::vector<int> ivFixed(BLOCK);
    for (int i = 0; i < BLOCK; ++i) {
        ivFixed[i] = (i * 11) & 0xFF;
    }

    // ----------------------------------------------------------------
    // 4️⃣  Iterate over keys and plaintexts
    // ----------------------------------------------------------------
    for (int keyIdx = 0; keyIdx < (int)keyBank.size(); ++keyIdx) {
        std::vector<int> curKey = keyBank[keyIdx];

        // generate round keys
        std::vector< std::vector<int> > roundKeySet;
        for (int r = 0; r < ROUNDS; ++r) {
            std::vector<int> rk(KEYLEN);
            for (int j = 0; j < KEYLEN; ++j) {
                rk[j] = curKey[j] ^ (r * 0x1F);
            }
            roundKeySet.push_back(rk);
        }

        for (int ptIdx = 0; ptIdx < (int)plainBank.size(); ++ptIdx) {
            std::vector<int> curPlain = plainBank[ptIdx];
            std::vector<int> feedback = ivFixed;
            std::vector<int> cipherText;

            int pos = 0;
            while (pos < (int)curPlain.size()) {
                std::vector<int> encBlock = feedback;
                for (int round = 0; round < ROUNDS; ++round) {
                    std::vector<int> curRoundKey = roundKeySet[round];
                    for (int byte = 0; byte < BLOCK; ++byte) {
                        encBlock[byte] ^= curRoundKey[byte];
                    }
                }

                int remain = (int)curPlain.size() - pos;
                int chunk = BLOCK;
                if (remain < BLOCK) chunk = remain;

                for (int k = 0; k < chunk; ++k) {
                    int ctByte = curPlain[pos + k] ^ encBlock[k];
                    cipherText.push_back(ctByte);

                    // shift feedback left by one byte
                    for (int shift = 0; shift < BLOCK - 1; ++shift) {
                        feedback[shift] = feedback[shift + 1];
                    }
                    feedback[BLOCK - 1] = ctByte;
                }
                pos += chunk;
            }

            std::cout << "KeyIdx=" << keyIdx
                      << " PlainIdx=" << ptIdx
                      << " CipherHex=";
            for (size_t out = 0; out < cipherText.size(); ++out) {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << (cipherText[out] & 0xFF);
            }
            std::cout << std::dec << std::endl;
        }
    }

    return 0;
}
