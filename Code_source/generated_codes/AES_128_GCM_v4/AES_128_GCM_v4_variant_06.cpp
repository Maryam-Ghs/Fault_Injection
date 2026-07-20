#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 6: ordered-structured */

class AES128GCM {
public:
    std::vector<int> keyBytes;          // 16 bytes
    std::vector<int> roundKey;          // 176 bytes (16 * 11)

    // S‑box
    int sbox[256] = {
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

    // Round constants
    int rcon[10] = {1,2,4,8,16,32,64,128,27,54};

    AES128GCM(const std::vector<int>& key) {
        keyBytes = key;
        keyExpansion();
    }

    // Rotate a 4‑byte word left by one byte
    void rotWord(std::vector<int>& w) {
        int tmp = w[0];
        w[0] = w[1];
        w[1] = w[2];
        w[2] = w[3];
        w[3] = tmp;
    }

    // Apply S‑box to each byte of a word
    void subWord(std::vector<int>& w) {
        int i = 0;
        while (i < 4) {
            w[i] = sbox[w[i]];
            i = i + 1;
        }
    }

    // Key schedule – produces 176‑byte round key
    void keyExpansion() {
        roundKey = std::vector<int>(176,0);
        int i = 0;
        while (i < 16) {
            roundKey[i] = keyBytes[i];
            i = i + 1;
        }
        i = 16;
        int r = 0;
        while (i < 176) {
            std::vector<int> temp(4);
            int j = 0;
            while (j < 4) {
                temp[j] = roundKey[i - 4 + j];
                j = j + 1;
            }
            if ((i / 4) % 4 == 0) {
                rotWord(temp);
                subWord(temp);
                temp[0] = temp[0] ^ rcon[r];
                r = r + 1;
            }
            j = 0;
            while (j < 4) {
                roundKey[i] = roundKey[i - 16] ^ temp[j];
                i = i + 1;
                j = j + 1;
            }
        }
    }

    // AddRoundKey step
    void addRoundKey(std::vector<int>& state, int round) {
        int i = 0;
        while (i < 16) {
            state[i] = state[i] ^ roundKey[round * 16 + i];
            i = i + 1;
        }
    }

    // SubBytes step
    void subBytes(std::vector<int>& state) {
        int i = 0;
        while (i < 16) {
            state[i] = sbox[state[i]];
            i = i + 1;
        }
    }

    // ShiftRows step (state is column‑major)
    void shiftRows(std::vector<int>& s) {
        std::vector<int> tmp(16);
        int r = 0;
        while (r < 4) {
            int c = 0;
            while (c < 4) {
                int src = ((c + r) % 4) * 4 + r;
                int dst = c * 4 + r;
                tmp[dst] = s[src];
                c = c + 1;
            }
            r = r + 1;
        }
        s = tmp;
    }

    // Multiply by 2 in GF(2^8)
    int xtime(int b) {
        int shifted = (b << 1) & 0xff;
        if (b & 0x80) {
            shifted = shifted ^ 0x1b;
        }
        return shifted;
    }

    // MixColumns step
    void mixColumns(std::vector<int>& s) {
        int c = 0;
        while (c < 4) {
            int i0 = c * 4;
            int a0 = s[i0];
            int a1 = s[i0+1];
            int a2 = s[i0+2];
            int a3 = s[i0+3];
            int r0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            int r1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            int r2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            int r3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
            s[i0]   = r0 & 0xff;
            s[i0+1] = r1 & 0xff;
            s[i0+2] = r2 & 0xff;
            s[i0+3] = r3 & 0xff;
            c = c + 1;
        }
    }

    // Encrypt a single 16‑byte block
    std::vector<int> encryptBlock(const std::vector<int>& block) {
        std::vector<int> state = block;
        addRoundKey(state,0);
        int round = 1;
        while (round < 10) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state,round);
            round = round + 1;
        }
        subBytes(state);
        shiftRows(state);
        addRoundKey(state,10);
        return state;
    }

    // XOR two byte vectors (same length)
    std::vector<int> xorVec(const std::vector<int>& a, const std::vector<int>& b) {
        std::vector<int> out(a.size(),0);
        int i = 0;
        while (i < (int)a.size()) {
            out[i] = a[i] ^ b[i];
            i = i + 1;
        }
        return out;
    }

    // Multiply two 128‑bit values in GF(2^128) (GHASH)
    std::vector<int> mulGF(const std::vector<int>& X, const std::vector<int>& Y) {
        std::vector<int> Z(16,0);
        std::vector<int> V = X;
        int i = 0;
        while (i < 128) {
            int byteIdx = i >> 3;
            int bitIdx  = 7 - (i & 7);
            int ybit = (Y[byteIdx] >> bitIdx) & 1;
            if (ybit) {
                Z = xorVec(Z,V);
            }
            // get LSB of V before shift
            int lsb = V[15] & 1;
            // shift V right by one bit
            int j = 15;
            while (j > 0) {
                V[j] = (V[j] >> 1) | ((V[j-1] & 1) << 7);
                j = j - 1;
            }
            V[0] = V[0] >> 1;
            if (lsb) {
                V[0] = V[0] ^ 0xe1;
            }
            i = i + 1;
        }
        return Z;
    }

    // Increment 32‑bit counter (last 4 bytes) – big endian
    void incCounter(std::vector<int>& ctr) {
        int i = 15;
        while (i >= 12) {
            ctr[i] = (ctr[i] + 1) & 0xff;
            if (ctr[i] != 0) break;
            i = i - 1;
        }
    }

    // GCM encryption (no AAD)
    void encrypt(const std::vector<int>& iv,
                 const std::vector<int>& plain,
                 std::vector<int>& cipher,
                 std::vector<int>& tag) {
        // H = E(K, 0^128)
        std::vector<int> zeroBlock(16,0);
        std::vector<int> H = encryptBlock(zeroBlock);

        // J0 = IV || 0x00000001
        std::vector<int> J0(16,0);
        int i = 0;
        while (i < 12) {
            J0[i] = iv[i];
            i = i + 1;
        }
        J0[15] = 1;   // last word = 1

        // Prepare counter
        std::vector<int> ctr = J0;

        // Encrypt blocks
        cipher.clear();
        int offset = 0;
        while (offset < (int)plain.size()) {
            incCounter(ctr);
            std::vector<int> keystream = encryptBlock(ctr);
            std::vector<int> block(16,0);
            int j = 0;
            while (j < 16) {
                block[j] = plain[offset + j];
                j = j + 1;
            }
            std::vector<int> ctBlock = xorVec(block,keystream);
            int k = 0;
            while (k < 16) {
                cipher.push_back(ctBlock[k]);
                k = k + 1;
            }
            offset = offset + 16;
        }

        // GHASH over ciphertext (no AAD)
        std::vector<int> Y(16,0);
        int pos = 0;
        while (pos < (int)cipher.size()) {
            std::vector<int> blk(16,0);
            int j = 0;
            while (j < 16) {
                blk[j] = cipher[pos + j];
                j = j + 1;
            }
            Y = xorVec(Y,blk);
            Y = mulGF(Y,H);
            pos = pos + 16;
        }
        // Length block: AAD bits = 0, ciphertext bits = len*8
        std::vector<int> lenBlock(16,0);
        long long cbits = ((long long)cipher.size()) * 8;
        // store cbits in low 64 bits (big endian)
        int idx = 8;
        while (idx < 16) {
            lenBlock[idx] = (int)((cbits >> (8*(15-idx))) & 0xff);
            idx = idx + 1;
        }
        Y = xorVec(Y,lenBlock);
        Y = mulGF(Y,H);

        // Tag = E(K, J0) xor Y
        std::vector<int> eJ0 = encryptBlock(J0);
        tag = xorVec(eJ0,Y);
    }
};

int main() {
    // Deterministic ordered inputs
    std::vector<int> key(16);
    for (int i = 0; i < 16; ++i) {
        key[i] = i; // 0,1,2,...,15
    }

    std::vector<int> iv(12);
    for (int i = 0; i < 12; ++i) {
        iv[i] = i + 0x10; // 0x10,0x11,...,0x1B
    }

    std::vector<int> plain(32);
    for (int i = 0; i < 32; ++i) {
        plain[i] = (i * 3) & 0xff; // patterned increasing values
    }

    AES128GCM engine(key);
    std::vector<int> cipher;
    std::vector<int> tag;
    engine.encrypt(iv,plain,cipher,tag);

    std::cout << "Ciphertext: ";
    for (size_t i = 0; i < cipher.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xff);
    }
    std::cout << std::dec << std::endl;

    std::cout << "Tag:        ";
    for (size_t i = 0; i < tag.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (tag[i] & 0xff);
    }
    std::cout << std::dec << std::endl;
    return 0;
}
