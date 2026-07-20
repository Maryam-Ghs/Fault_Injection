#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */
int main() {
    // ------------------------------------------------------------
    // ARIA-CFB (Version #6) – sparse and skewed test data.
    // ------------------------------------------------------------

    int BLOCK = 16;
    int KEYLEN = 16;
    int ROUNDS = 2;

    // ----------------------------------------------------------------
    // 1️⃣  Many keys, but most have low‑entropy (mostly zeros)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > keyBank;

    // high‑entropy key
    std::vector<int> highKey(KEYLEN);
    int i = 0;
    while (i < KEYLEN) {
        highKey[i] = (i * 23) & 0xFF;
        i = i + 1;
    }
    keyBank.push_back(highKey);

    // fifteen low‑entropy keys (mostly zeros, a few ones)
    i = 0;
    while (i < 15) {
        std::vector<int> lowKey(KEYLEN, 0);
        if (i % 3 == 0) lowKey[0] = 1;   // occasional 1 at start
        keyBank.push_back(lowKey);
        i = i + 1;
    }

    // ----------------------------------------------------------------
    // 2️⃣  Plaintexts skewed towards short lengths, with a few long ones
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > plainBank;

    // many short (1‑byte) plaintexts
    i = 0;
    while (i < 10) {
        plainBank.push_back(std::vector<int>(1, i & 0xFF));
        i = i + 1;
    }

    // a few medium (8‑byte) plaintexts
    i = 0;
    while (i < 3) {
        std::vector<int> med(8);
        int j = 0;
        while (j < 8) {
            med[j] = (j * 13 + i) & 0xFF;
            j = j + 1;
        }
        plainBank.push_back(med);
        i = i + 1;
    }

    // two long (64‑byte) plaintexts
    i = 0;
    while (i < 2) {
        std::vector<int> longP(64);
        int j = 0;
        while (j < 64) {
            longP[j] = (j * 7 + i) & 0xFF;
            j = j + 1;
        }
        plainBank.push_back(longP);
        i = i + 1;
    }

    // ----------------------------------------------------------------
    // 3️⃣  Fixed IV
    // ----------------------------------------------------------------
    std::vector<int> ivFixed(BLOCK);
    i = 0;
    while (i < BLOCK) {
        ivFixed[i] = (i * 11) & 0xFF;
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
