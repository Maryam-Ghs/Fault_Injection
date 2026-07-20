#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 4: signed-extremes */
int main() {
    // ------------------------------------------------------------
    // ARIA-CFB (Version #6) – self‑contained test with signed extremes.
    // ------------------------------------------------------------

    int BLOCK = 16;
    int KEYLEN = 16;
    int ROUNDS = 3;

    // ----------------------------------------------------------------
    // 1️⃣  Build a collection of keys with signed extremes
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > keyBank;

    // all zeros
    keyBank.push_back(std::vector<int>(KEYLEN, 0));

    // all -1 (0xFFFFFFFF when masked)
    keyBank.push_back(std::vector<int>(KEYLEN, -1));

    // mixed signed values: -128, -64, -1, 0, 1, 64, 127, 255, rest zero
    std::vector<int> keyMixed(KEYLEN, 0);
    int i = 0;
    while (i < KEYLEN) {
        if (i == 0) keyMixed[i] = -128;
        else if (i == 1) keyMixed[i] = -64;
        else if (i == 2) keyMixed[i] = -1;
        else if (i == 3) keyMixed[i] = 0;
        else if (i == 4) keyMixed[i] = 1;
        else if (i == 5) keyMixed[i] = 64;
        else if (i == 6) keyMixed[i] = 127;
        else if (i == 7) keyMixed[i] = 255;
        else keyMixed[i] = 0;
        i = i + 1;
    }
    keyBank.push_back(keyMixed);

    // ----------------------------------------------------------------
    // 2️⃣  Build plaintexts with signed extremes
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > plainBank;

    // empty
    plainBank.push_back(std::vector<int>());

    // all -1 (0xFFFFFFFF)
    plainBank.push_back(std::vector<int>(BLOCK, -1));

    // alternating -128 / 127
    std::vector<int> altPlain(BLOCK);
    i = 0;
    while (i < BLOCK) {
        altPlain[i] = (i % 2 == 0) ? -128 : 127;
        i = i + 1;
    }
    plainBank.push_back(altPlain);

    // single byte -64
    plainBank.push_back(std::vector<int>(1, -64));

    // 20‑byte mixed pattern
    std::vector<int> mixed20(20);
    i = 0;
    while (i < 20) {
        mixed20[i] = (i % 5 == 0) ? -100 : (i * 11) & 0xFF;
        i = i + 1;
    }
    plainBank.push_back(mixed20);

    // ----------------------------------------------------------------
    // 3️⃣  Fixed IV
    // ----------------------------------------------------------------
    std::vector<int> ivFixed(BLOCK);
    i = 0;
    while (i < BLOCK) {
        ivFixed[i] = (i * 13) & 0xFF;
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
                    int ptByte = curPlain[pos + k];
                    int ctByte = ptByte ^ encBlock[k];
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
