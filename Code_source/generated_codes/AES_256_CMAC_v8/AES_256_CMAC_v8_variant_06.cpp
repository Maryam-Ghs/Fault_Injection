/ * LLM input variant 6: ordered-structured * /
#include <iostream>
#include <vector>
#include <iomanip>

class AES256CMAC {
    std::vector<int> masterKey;               // 32 bytes
    std::vector< std::vector<int> > roundKeys; // 15 round keys (16 bytes each)

    // S‑box (0‑255)
    static int sbox[256];
    // Rcon for key schedule
    static int rcon[15];

    // ---------- finite‑field helpers ----------
    static int xtime(int v) {
        int shifted = (v << 1) & 0xFF;
        return (v & 0x80) ? (shifted ^ 0x1B) : shifted;
    }
    static int mul2(int v) { return xtime(v); }
    static int mul3(int v) { return xtime(v) ^ v; }

    // ---------- core AES operations ----------
    void subBytes(std::vector<int>& st) {
        int i = 0;
        while (i < 16) st[i] = sbox[st[i]], ++i;
    }
    void shiftRows(std::vector<int>& st) {
        int t;
        // row 1
        t = st[1]; st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = t;
        // row 2
        t = st[2]; st[2] = st[10]; st[10] = t;
        t = st[6]; st[6] = st[14]; st[14] = t;
        // row 3
        t = st[3]; st[3] = st[15]; st[15] = st[11]; st[11] = st[7]; st[7] = t;
    }
    void mixColumns(std::vector<int>& st) {
        int i = 0;
        while (i < 16) {
            int a0 = st[i], a1 = st[i+1], a2 = st[i+2], a3 = st[i+3];
            st[i]   = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            st[i+1] = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            st[i+2] = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            st[i+3] = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            i += 4;
        }
    }
    void addRoundKey(std::vector<int>& st, const std::vector<int>& rk) {
        int i = 0;
        while (i < 16) st[i] ^= rk[i], ++i;
    }

    // ---------- key schedule ----------
    void expandKey() {
        roundKeys.clear();
        roundKeys.push_back(std::vector<int>(masterKey.begin(), masterKey.begin()+16));
        roundKeys.push_back(std::vector<int>(masterKey.begin()+16, masterKey.end()));

        int i = 8; // word index (32‑bit words)
        while (i < 60) {
            std::vector<int> temp(4);
            // copy previous word
            for (int j = 0; j < 4; ++j) temp[j] = roundKeys.back()[j];
            if (i % 8 == 0) {
                // RotWord
                int rot = temp[0];
                temp[0] = temp[1]; temp[1] = temp[2]; temp[2] = temp[3]; temp[3] = rot;
                // SubWord
                for (int j = 0; j < 4; ++j) temp[j] = sbox[temp[j]];
                // Rcon
                temp[0] ^= rcon[i/8];
            } else if (i % 8 == 4) {
                for (int j = 0; j < 4; ++j) temp[j] = sbox[temp[j]];
            }
            // XOR with word 8 positions back
            std::vector<int> prev = roundKeys[i-8];
            std::vector<int> next(16);
            for (int j = 0; j < 4; ++j) next[j] = prev[j] ^ temp[j];
            // copy rest of the 12 bytes from previous round key
            for (int j = 4; j < 16; ++j) next[j] = roundKeys[i-8][j] ^ roundKeys.back()[j];
            roundKeys.push_back(next);
            ++i;
        }
    }

    // ---------- single block encryption ----------
    void encryptBlock(const std::vector<int>& src, std::vector<int>& dst) {
        dst = src;
        addRoundKey(dst, roundKeys[0]);
        int r = 1;
        while (r < 14) {
            subBytes(dst);
            shiftRows(dst);
            mixColumns(dst);
            addRoundKey(dst, roundKeys[r]);
            ++r;
        }
        subBytes(dst);
        shiftRows(dst);
        addRoundKey(dst, roundKeys[14]);
    }

    // ---------- subkey generation for CMAC ----------
    void generateSubkeys(std::vector<int>& k1, std::vector<int>& k2) {
        std::vector<int> zero(16, 0);
        std::vector<int> l(16);
        encryptBlock(zero, l);
        // left shift by 1
        int carry = 0;
        for (int i = 15; i >= 0; --i) {
            int nxt = (l[i] << 1) & 0xFF;
            k1[i] = nxt | carry;
            carry = (l[i] & 0x80) ? 1 : 0;
        }
        if (carry) k1[15] ^= 0x87;
        // k2 = k1 << 1
        carry = 0;
        for (int i = 15; i >= 0; --i) {
            int nxt = (k1[i] << 1) & 0xFF;
            k2[i] = nxt | carry;
            carry = (k1[i] & 0x80) ? 1 : 0;
        }
        if (carry) k2[15] ^= 0x87;
    }

public:
    AES256CMAC(const std::vector<int>& key) : masterKey(key) {
        expandKey();
    }

    std::vector<int> compute(const std::vector<int>& msg) {
        std::vector<int> k1(16), k2(16);
        generateSubkeys(k1, k2);
        int blocks = (msg.size() + 15) / 16;
        bool lastComplete = (msg.size() % 16) == 0 && msg.size() != 0;
        std::vector<int> lastBlock(16, 0);
        if (lastComplete) {
            for (int i = 0; i < 16; ++i) lastBlock[i] = msg[(blocks-1)*16 + i] ^ k1[i];
        } else {
            int len = msg.size() % 16;
            for (int i = 0; i < len; ++i) lastBlock[i] = msg[(blocks-1)*16 + i];
            lastBlock[len] = 0x80;
            for (int i = 0; i < 16; ++i) lastBlock[i] ^= k2[i];
        }
        std::vector<int> x(16, 0);
        for (int b = 0; b < blocks-1; ++b) {
            std::vector<int> y(16);
            for (int i = 0; i < 16; ++i) y[i] = msg[b*16 + i] ^ x[i];
            encryptBlock(y, x);
        }
        // final
        std::vector<int> y(16);
        for (int i = 0; i < 16; ++i) y[i] = lastBlock[i] ^ x[i];
        encryptBlock(y, x);
        return x;
    }
};

// ---------- static tables ----------
int AES256CMAC::sbox[256] = {
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

int AES256CMAC::rcon[15] = {
  0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8,0xAB,0x4D,0x9A
};

int main() {
    // Structured test input: sequential key and message bytes
    std::vector<int> key = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F
    };
    // Message of three 16‑byte blocks with ascending values 0x00..0x2F
    std::vector<int> msg = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
        0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
        0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F
    };

    AES256CMAC algo(key);
    std::vector<int> tag = algo.compute(msg);

    std::cout << "CMAC: ";
    for (int b : tag) std::cout << std::hex << std::setw(2) << std::setfill('0') << b;
    std::cout << std::dec << std::endl;
    return 0;
}
