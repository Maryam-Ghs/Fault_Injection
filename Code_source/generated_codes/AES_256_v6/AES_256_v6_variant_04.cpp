#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 4: signed-extremes */

class AES256 {
    std::vector<int> sboxTbl;
    std::vector<int> rconTbl;
    std::vector<int> expandedKeys;   // 240 bytes for 14 rounds + initial

public:
    AES256(std::vector<int> key) {
        initSbox();
        initRcon();
        keyExpand(key);
    }

    void encryptBlock(const std::vector<int>& plain, std::vector<int>& cipher) {
        // copy plaintext into state (16 bytes)
        std::vector<int> state(16);
        int idx = 0;
        while (idx < 16) {
            state[idx] = plain[idx] & 0xFF;
            idx = idx + 1;
        }

        // initial AddRoundKey
        idx = 0;
        while (idx < 16) {
            state[idx] ^= expandedKeys[idx];
            idx = idx + 1;
        }

        int round = 1;
        while (round < 14) {
            // ----- SubBytes -----
            idx = 0;
            while (idx < 16) {
                state[idx] = sboxTbl[state[idx]];
                idx = idx + 1;
            }

            // ----- ShiftRows -----
            // row 1 shift left by 1
            int tmp = state[1];
            state[1]  = state[5];
            state[5]  = state[9];
            state[9]  = state[13];
            state[13] = tmp;
            // row 2 shift left by 2
            tmp = state[2];
            int tmp2 = state[6];
            state[2]  = state[10];
            state[6]  = state[14];
            state[10] = tmp;
            state[14] = tmp2;
            // row 3 shift left by 3 (right by 1)
            tmp = state[3];
            state[3]  = state[15];
            state[15] = state[11];
            state[11] = state[7];
            state[7]  = tmp;

            // ----- MixColumns -----
            idx = 0;
            while (idx < 16) {
                int a0 = state[idx];
                int a1 = state[idx + 1];
                int a2 = state[idx + 2];
                int a3 = state[idx + 3];

                int b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
                int b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
                int b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
                int b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);

                state[idx]     = b0 & 0xFF;
                state[idx + 1] = b1 & 0xFF;
                state[idx + 2] = b2 & 0xFF;
                state[idx + 3] = b3 & 0xFF;

                idx = idx + 4;
            }

            // ----- AddRoundKey -----
            idx = 0;
            int keyOff = round * 16;
            while (idx < 16) {
                state[idx] ^= expandedKeys[keyOff + idx];
                idx = idx + 1;
            }

            round = round + 1;
        }

        // ----- Final round (no MixColumns) -----
        // SubBytes
        idx = 0;
        while (idx < 16) {
            state[idx] = sboxTbl[state[idx]];
            idx = idx + 1;
        }
        // ShiftRows
        // row 1
        int tmp = state[1];
        state[1]  = state[5];
        state[5]  = state[9];
        state[9]  = state[13];
        state[13] = tmp;
        // row 2
        tmp = state[2];
        int tmp2 = state[6];
        state[2]  = state[10];
        state[6]  = state[14];
        state[10] = tmp;
        state[14] = tmp2;
        // row 3
        tmp = state[3];
        state[3]  = state[15];
        state[15] = state[11];
        state[11] = state[7];
        state[7]  = tmp;
        // AddRoundKey
        idx = 0;
        int finalOff = 14 * 16;
        while (idx < 16) {
            state[idx] ^= expandedKeys[finalOff + idx];
            idx = idx + 1;
        }

        // copy to output
        cipher.resize(16);
        idx = 0;
        while (idx < 16) {
            cipher[idx] = state[idx];
            idx = idx + 1;
        }
    }

private:
    void initSbox() {
        int raw[256] = {
            99,124,119,123,242,107,111,197,48,1,103,43,254,215,171,118,
            202,130,201,125,250,89,71,240,173,212,162,175,156,164,114,192,
            183,253,147,38,54,63,247,204,52,165,229,241,113,216,49,21,
            4,199,35,195,24,150,5,154,7,18,128,226,235,39,178,117,
            9,131,44,26,27,110,90,160,82,59,214,179,41,227,47,132,
            83,209,0,237,32,252,177,91,106,203,190,57,74,76,88,207,
            208,239,170,251,67,77,51,133,69,249,2,127,80,60,159,168,
            81,163,64,143,146,157,56,245,188,182,218,33,16,255,243,210,
            205,12,19,236,95,151,68,23,196,167,126,61,100,93,25,115,
            96,129,79,220,34,42,144,136,70,238,184,20,222,94,11,219,
            224,50,58,10,73,6,36,92,194,211,172,98,145,149,228,121,
            231,200,55,109,141,213,78,169,108,86,244,234,101,122,174,8,
            186,120,37,46,28,166,180,198,232,221,116,31,75,189,139,138,
            112,62,181,102,72,3,246,14,97,53,87,185,134,193,29,158,
            225,248,152,17,105,217,142,148,155,30,135,233,206,85,40,223,
            140,161,137,13,191,230,66,104,65,153,45,15,176,84,187,22
        };
        sboxTbl.assign(raw, raw + 256);
    }

    void initRcon() {
        rconTbl.resize(15);
        int c = 1;
        int i = 1;
        while (i < 15) {
            rconTbl[i] = c;
            c = mul2(c);
            i = i + 1;
        }
    }

    void keyExpand(std::vector<int> key) {
        // key is 32 bytes
        expandedKeys.resize(240);
        int i = 0;
        while (i < 32) {
            expandedKeys[i] = key[i] & 0xFF;
            i = i + 1;
        }
        int bytesGenerated = 32;
        int rconIter = 1;
        while (bytesGenerated < 240) {
            // take previous 4 bytes
            int t0 = expandedKeys[bytesGenerated - 4];
            int t1 = expandedKeys[bytesGenerated - 3];
            int t2 = expandedKeys[bytesGenerated - 2];
            int t3 = expandedKeys[bytesGenerated - 1];

            // every 8th word apply schedule core
            if ((bytesGenerated % 32) == 0) {
                // RotWord
                int tmp = t0;
                t0 = t1;
                t1 = t2;
                t2 = t3;
                t3 = tmp;
                // SubWord
                t0 = sboxTbl[t0 & 0xFF];
                t1 = sboxTbl[t1 & 0xFF];
                t2 = sboxTbl[t2 & 0xFF];
                t3 = sboxTbl[t3 & 0xFF];
                // Rcon
                t0 ^= rconTbl[rconIter];
                rconIter = rconIter + 1;
            }
            // every 16th byte (i.e., when bytesGenerated % 32 == 16) apply SubWord
            if ((bytesGenerated % 32) == 16) {
                t0 = sboxTbl[t0 & 0xFF];
                t1 = sboxTbl[t1 & 0xFF];
                t2 = sboxTbl[t2 & 0xFF];
                t3 = sboxTbl[t3 & 0xFF];
            }

            // XOR with word Nk positions earlier (8 words = 32 bytes)
            expandedKeys[bytesGenerated]     = (expandedKeys[bytesGenerated - 32] ^ t0) & 0xFF;
            expandedKeys[bytesGenerated + 1] = (expandedKeys[bytesGenerated - 31] ^ t1) & 0xFF;
            expandedKeys[bytesGenerated + 2] = (expandedKeys[bytesGenerated - 30] ^ t2) & 0xFF;
            expandedKeys[bytesGenerated + 3] = (expandedKeys[bytesGenerated - 29] ^ t3) & 0xFF;

            bytesGenerated = bytesGenerated + 4;
        }
    }

    // Galois multiplication helpers (only need mul2 and mul3)
    int mul2(int x) {
        int shifted = (x << 1) & 0xFF;
        if (x & 0x80) shifted ^= 0x1B;
        return shifted;
    }
    int mul3(int x) {
        return mul2(x) ^ x;
    }
};

int main() {
    std::srand((int)std::time(0));

    // deterministic key with signed extremes
    int rawKey[32] = {
        -128, -64, -1, 0, 1, 2, 3, 4,
        5, 6, 7, 8, 9, 10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, 26, 27, 28
    };
    std::vector<int> keyVec(32);
    for (int i = 0; i < 32; ++i) {
        keyVec[i] = rawKey[i];
    }

    // deterministic plaintext mixing zero and positive values
    int rawPlain[16] = {
        0, 1, 2, 3,
        0, 127, 200, 255,
        0, 15, 16, 17,
        18, 19, 20, 21
    };
    std::vector<int> plainVec(16);
    for (int i = 0; i < 16; ++i) {
        plainVec[i] = rawPlain[i];
    }

    AES256 cipherEngine(keyVec);
    std::vector<int> cipherVec;
    cipherEngine.encryptBlock(plainVec, cipherVec);

    // print results
    std::cout << "Key (hex): ";
    for (int i = 0; i < 32; ++i) {
        std::cout << std::hex << ((keyVec[i] & 0xFF) < 16 ? "0" : "") << (keyVec[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;

    std::cout << "Plaintext (hex): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << (plainVec[i] < 16 ? "0" : "") << plainVec[i];
    }
    std::cout << std::dec << std::endl;

    std::cout << "Ciphertext (hex): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << (cipherVec[i] < 16 ? "0" : "") << cipherVec[i];
    }
    std::cout << std::dec << std::endl;

    return 0;
}
