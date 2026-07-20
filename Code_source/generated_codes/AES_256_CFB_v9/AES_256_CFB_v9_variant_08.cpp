/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <vector>
#include <iomanip>

// ------------------------------------------------------------
// AES-256 in CFB mode (branch‑minimized, class‑based)
// ------------------------------------------------------------
class Cipher256CFB {
public:
    // constructor takes 256‑bit key and 128‑bit IV (both as int vectors)
    Cipher256CFB(std::vector<int> &keyBytes, std::vector<int> &ivBytes) {
        // copy key and IV (int values 0‑255)
        for (int i = 0; i < 32; ++i) keyMat[i] = keyBytes[i];
        for (int i = 0; i < 16; ++i) ivReg[i] = ivBytes[i];
        // key schedule
        expandKey();
    }

    // encrypt a plaintext vector (multiple of 16 bytes) into ciphertext vector
    void encrypt(const std::vector<int> &plain, std::vector<int> &cipher) {
        int blocks = (int)plain.size() / 16;
        for (int b = 0; b < blocks; ++b) {
            // generate keystream block
            std::vector<int> keystream(16);
            encryptBlock(ivReg, keystream);
            // XOR with plaintext to obtain ciphertext block
            for (int i = 0; i < 16; ++i) {
                int ct = plain[b*16 + i] ^ keystream[i];
                cipher.push_back(ct);
                ivReg[i] = ct;                     // feedback for next block
            }
        }
    }

private:
    // --------------------------------------------------------------------
    // internal data
    // --------------------------------------------------------------------
    int keyMat[32];          // 256‑bit key (32 bytes)
    int roundKey[60][4];     // 60 words (4‑byte each) = 240 bytes
    int ivReg[16];           // current IV / feedback register

    // --------------------------------------------------------------------
    // static tables (non‑const as required)
    // --------------------------------------------------------------------
    int sBox[256];
    int rCon[15];

    // --------------------------------------------------------------------
    // helper: multiply by 2 in GF(2^8)
    // --------------------------------------------------------------------
    inline int xtime(int x) {
        int shifted = (x << 1) & 0xFF;
        int reduced = (x & 0x80) ? (shifted ^ 0x1B) : shifted;
        return reduced;
    }

    // --------------------------------------------------------------------
    // MixColumns transformation (standard direct computation)
    // --------------------------------------------------------------------
    void mixColumn(int *col) {
        int a0 = col[0], a1 = col[1], a2 = col[2], a3 = col[3];
        int t = a0 ^ a1 ^ a2 ^ a3;
        int u = a0;
        col[0] ^= t ^ xtime(a0 ^ a1);
        col[1] ^= t ^ xtime(a1 ^ a2);
        col[2] ^= t ^ xtime(a2 ^ a3);
        col[3] ^= t ^ xtime(a3 ^ u);
    }

    // --------------------------------------------------------------------
    // Core AES block encryption (16‑byte state)
    // --------------------------------------------------------------------
    void encryptBlock(int *inBlock, std::vector<int> &outBlock) {
        int state[4][4];
        // copy input to state (column‑major)
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                state[r][c] = inBlock[c*4 + r];

        addRoundKey(state, 0);

        for (int round = 1; round < 14; ++round) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 14);

        // write state to output vector (column‑major)
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                outBlock.push_back(state[r][c] & 0xFF);
    }

    // --------------------------------------------------------------------
    // SubBytes using sBox table
    // --------------------------------------------------------------------
    void subBytes(int state[4][4]) {
        for (int i = 0; i < 16; ++i) {
            int row = i & 3;
            int col = i >> 2;
            state[row][col] = sBox[state[row][col]];
        }
    }

    // --------------------------------------------------------------------
    // ShiftRows (standard)
    // --------------------------------------------------------------------
    void shiftRows(int state[4][4]) {
        int tmp;
        // row 1: shift left 1
        tmp = state[1][0];
        state[1][0] = state[1][1];
        state[1][1] = state[1][2];
        state[1][2] = state[1][3];
        state[1][3] = tmp;
        // row 2: shift left 2
        tmp = state[2][0];
        state[2][0] = state[2][2];
        state[2][2] = tmp;
        tmp = state[2][1];
        state[2][1] = state[2][3];
        state[2][3] = tmp;
        // row 3: shift left 3 (right 1)
        tmp = state[3][3];
        state[3][3] = state[3][2];
        state[3][2] = state[3][1];
        state[3][1] = state[3][0];
        state[3][0] = tmp;
    }

    // --------------------------------------------------------------------
    // MixColumns (apply to each column)
    // --------------------------------------------------------------------
    void mixColumns(int state[4][4]) {
        for (int c = 0; c < 4; ++c) {
            int col[4] = { state[0][c], state[1][c], state[2][c], state[3][c] };
            mixColumn(col);
            for (int r = 0; r < 4; ++r) state[r][c] = col[r];
        }
    }

    // --------------------------------------------------------------------
    // AddRoundKey (XOR with round key)
    // --------------------------------------------------------------------
    void addRoundKey(int state[4][4], int round) {
        for (int c = 0; c < 4; ++c) {
            for (int r = 0; r < 4; ++r) {
                int word = roundKey[round*4 + c][r];
                state[r][c] ^= word;
            }
        }
    }

    // --------------------------------------------------------------------
    // Key expansion for AES‑256 (60 words)
    // --------------------------------------------------------------------
    void expandKey() {
        // initialise static tables (filled once)
        initTables();

        // first 8 words are the key itself
        for (int i = 0; i < 8; ++i) {
            roundKey[i][0] = keyMat[4*i];
            roundKey[i][1] = keyMat[4*i+1];
            roundKey[i][2] = keyMat[4*i+2];
            roundKey[i][3] = keyMat[4*i+3];
        }

        for (int i = 8; i < 60; ++i) {
            int temp[4];
            // copy previous word
            for (int j = 0; j < 4; ++j) temp[j] = roundKey[i-1][j];

            // every 8th word: RotWord + SubWord + Rcon
            int cond = (i % 8 == 0);
            int rot[4];
            // RotWord (rotate left)
            rot[0] = temp[1]; rot[1] = temp[2]; rot[2] = temp[3]; rot[3] = temp[0];
            // SubWord (apply S‑box)
            for (int j = 0; j < 4; ++j) rot[j] = sBox[rot[j]];
            // Rcon addition
            rot[0] ^= rCon[i/8];
            // select based on condition without branch
            for (int j = 0; j < 4; ++j) temp[j] = cond ? rot[j] : temp[j];

            // every 4th word after the 8th: SubWord only
            cond = (i % 8 == 4);
            for (int j = 0; j < 4; ++j) {
                int sb = sBox[temp[j]];
                temp[j] = cond ? sb : temp[j];
            }

            // XOR with word[i‑8]
            for (int j = 0; j < 4; ++j)
                roundKey[i][j] = roundKey[i-8][j] ^ temp[j];
        }
    }

    // --------------------------------------------------------------------
    // Initialise S‑box and Rcon (non‑const arrays)
    // --------------------------------------------------------------------
    void initTables() {
        // S‑box values
        int sb[256] = {
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
        for (int i = 0; i < 256; ++i) sBox[i] = sb[i];

        // Rcon values (only first 14 needed)
        int rc[15] = {0,1,2,4,8,16,32,64,128,27,54,108,216,171,205};
        for (int i = 0; i < 15; ++i) rCon[i] = rc[i];
    }
};

// ------------------------------------------------------------
// Helper: print a vector as hex bytes
// ------------------------------------------------------------
void dumpHex(const std::vector<int> &data, const char *label) {
    std::cout << label << ": ";
    for (int i = 0; i < (int)data.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (data[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;
}

// ------------------------------------------------------------
// Deterministic test vector (AES‑256‑CFB)
// ------------------------------------------------------------
int main() {
    // 256‑bit key (32 bytes) - sparse, mostly zeros
    std::vector<int> keyBytes = {
        0x01,0x02,0x03,0x04, 0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
    };
    // 128‑bit IV (16 bytes) - sparse
    std::vector<int> ivBytes = {
        0xAA,0xBB,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };
    // Plaintext (two blocks) - sparse, clustered non‑zeros
    std::vector<int> plainBytes = {
        0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xAA
    };

    dumpHex(keyBytes, "Key");
    dumpHex(ivBytes, "IV");
    dumpHex(plainBytes, "Plaintext");

    // encrypt
    Cipher256CFB engine(keyBytes, ivBytes);
    std::vector<int> cipherBytes;
    engine.encrypt(plainBytes, cipherBytes);

    dumpHex(cipherBytes, "Ciphertext");

    return 0;
}
