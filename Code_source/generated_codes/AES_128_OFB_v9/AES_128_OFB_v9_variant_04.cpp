/* LLM input variant 4: signed-extremes */
/*  AES-128-OFB implementation – version #9
    Requirements:
    * only int / float types (no double, long, unsigned, const)
    * class‑based, std::vector usage
    * internal random input, no cin
    * simple, readable style
*/

#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <ctime>

class AesOfb {
public:
    AesOfb(const std::vector<int>& keyBytes) {
        // store original key (16 bytes)
        masterKey = keyBytes;
        expandKey();
    }

    // OFB encryption: returns ciphertext vector (same length as plaintext)
    std::vector<int> encrypt(const std::vector<int>& plain) {
        std::vector<int> out(plain.size());
        std::vector<int> iv(16);
        // simple IV: first 16 bytes of key (just for demo)
        for (int i = 0; i < 16; ++i) iv[i] = masterKey[i];

        int blockCount = (int)plain.size() / 16;
        int leftover   = (int)plain.size() % 16;

        for (int b = 0; b < blockCount; ++b) {
            iv = encryptBlock(iv);                     // produce keystream block
            for (int i = 0; i < 16; ++i) {
                out[b * 16 + i] = iv[i] ^ plain[b * 16 + i];
            }
        }
        // handle possible final partial block
        if (leftover > 0) {
            iv = encryptBlock(iv);
            for (int i = 0; i < leftover; ++i) {
                out[blockCount * 16 + i] = iv[i] ^ plain[blockCount * 16 + i];
            }
        }
        return out;
    }

private:
    std::vector<int> masterKey;          // 16 bytes
    std::vector<int> roundKeys;          // 176 bytes (11 * 16)

    // -----------------------------------------------------------------
    //  S‑box (static, 256 entries, values 0‑255)
    // -----------------------------------------------------------------
    static int sbox[256];
    static int rcon[11];

    // -----------------------------------------------------------------
    //  Key expansion (AES‑128, 10 rounds)
    // -----------------------------------------------------------------
    void expandKey() {
        roundKeys.resize(176);
        // copy initial key
        for (int i = 0; i < 16; ++i) roundKeys[i] = masterKey[i];

        int bytesGenerated = 16;
        int rconIdx = 1;
        while (bytesGenerated < 176) {
            // take last 4 bytes
            int temp0 = roundKeys[bytesGenerated - 4];
            int temp1 = roundKeys[bytesGenerated - 3];
            int temp2 = roundKeys[bytesGenerated - 2];
            int temp3 = roundKeys[bytesGenerated - 1];

            // rotate word every 16 bytes
            if (bytesGenerated % 16 == 0) {
                // RotWord
                int t = temp0;
                temp0 = temp1; temp1 = temp2; temp2 = temp3; temp3 = t;
                // SubWord
                temp0 = sbox[temp0]; temp1 = sbox[temp1];
                temp2 = sbox[temp2]; temp3 = sbox[temp3];
                // Rcon
                temp0 ^= rcon[rconIdx];
                ++rconIdx;
            }

            // XOR with word 16 bytes before
            for (int i = 0; i < 4; ++i) {
                int prev = roundKeys[bytesGenerated - 16 + i];
                int val  = (i == 0 ? temp0 : (i == 1 ? temp1 : (i == 2 ? temp2 : temp3)));
                roundKeys[bytesGenerated] = prev ^ val;
                ++bytesGenerated;
            }
        }
    }

    // -----------------------------------------------------------------
    //  One AES block encryption (ECB) – returns 16‑byte vector
    // -----------------------------------------------------------------
    std::vector<int> encryptBlock(const std::vector<int>& inBlock) {
        std::vector<int> state = inBlock;                 // 16 bytes

        addRoundKey(state, 0);

        for (int round = 1; round <= 9; ++round) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
        }

        // final round (no MixColumns)
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 10);

        return state;
    }

    // -----------------------------------------------------------------
    //  Core AES transformations (all work on 16‑byte vector)
    // -----------------------------------------------------------------
    void subBytes(std::vector<int>& st) {
        for (int i = 0; i < 16; ++i) st[i] = sbox[st[i]];
    }

    void shiftRows(std::vector<int>& st) {
        // row 1 – shift left 1
        int tmp = st[1];
        st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = tmp;
        // row 2 – shift left 2
        int tmp1 = st[2]; int tmp2 = st[6];
        st[2] = st[10]; st[6] = st[14]; st[10] = tmp1; st[14] = tmp2;
        // row 3 – shift left 3 (right 1)
        tmp = st[15];
        st[15] = st[11]; st[11] = st[7]; st[7] = st[3]; st[3] = tmp;
    }

    void mixColumns(std::vector<int>& st) {
        for (int c = 0; c < 4; ++c) {
            int i0 = c * 4;
            int a0 = st[i0];
            int a1 = st[i0 + 1];
            int a2 = st[i0 + 2];
            int a3 = st[i0 + 3];

            int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);

            st[i0]     = r0;
            st[i0 + 1] = r1;
            st[i0 + 2] = r2;
            st[i0 + 3] = r3;
        }
    }

    void addRoundKey(std::vector<int>& st, int round) {
        int start = round * 16;
        for (int i = 0; i < 16; ++i) st[i] ^= roundKeys[start + i];
    }

    // -----------------------------------------------------------------
    //  Finite‑field multiplication helpers (AES‑GF(2^8))
    // -----------------------------------------------------------------
    int mul2(int x) {
        int shifted = x << 1;
        if (x & 0x80) shifted ^= 0x1b;
        return shifted & 0xff;
    }
    int mul3(int x) { return mul2(x) ^ x; }
};

// ---------------------------------------------------------------------
//  Static tables (definition outside class)
// ---------------------------------------------------------------------
int AesOfb::sbox[256] = {
    -128, 0, 127, -1, 255, -64, 64, -32, 32, -16, 16, -8, 8, -4, 4, -2,
    2, -3, 3, -5, 5, -7, 7, -9, 9, -11, 11, -13, 13, -15, 15, -17,
    -18, 18, -19, 19, -20, 20, -21, 21, -22, 22, -23, 23, -24, 24, -25, 25,
    -26, 26, -27, 27, -28, 28, -29, 29, -30, 30, -31, 31, -33, 33, -34, 34,
    -35, 35, -36, 36, -37, 37, -38, 38, -39, 39, -40, 40, -41, 41, -42, 42,
    -43, 43, -44, 44, -45, 45, -46, 46, -47, 47, -48, 48, -49, 49, -50, 50,
    -51, 51, -52, 52, -53, 53, -54, 54, -55, 55, -56, 56, -57, 57, -58, 58,
    -59, 59, -60, 60, -61, 61, -62, 62, -63, 63, -65, 65, -66, 66, -67, 67,
    -68, 68, -69, 69, -70, 70, -71, 71, -72, 72, -73, 73, -74, 74, -75, 75,
    -76, 76, -77, 77, -78, 78, -79, 79, -80, 80, -81, 81, -82, 82, -83, 83,
    -84, 84, -85, 85, -86, 86, -87, 87, -88, 88, -89, 89, -90, 90, -91, 91,
    -92, 92, -93, 93, -94, 94, -95, 95, -96, 96, -97, 97, -98, 98, -99, 99,
    -100, 100, -101, 101, -102, 102, -103, 103, -104, 104, -105, 105, -106, 106, -107, 107,
    -108, 108, -109, 109, -110, 110, -111, 111, -112, 112, -113, 113, -114, 114, -115, 115,
    -116, 116, -117, 117, -118, 118, -119, 119, -120, 120, -121, 121, -122, 122, -123, 123,
    -124, 124, -125, 125, -126, 126, -127, 127, 0, 0, 0, 0, 0, 0, 0, 0,
    // fill remaining entries with a repeating pattern of extremes
    -128, 127, -1, 0, 1, -2, 2, -3, 3, -4, 4, -5, 5, -6, 6, -7,
    7, -8, 8, -9, 9, -10, 10, -11, 11, -12, 12, -13, 13, -14, 14, -15,
    15, -16, 16, -17, 17, -18, 18, -19, 19, -20, 20, -21, 21, -22, 22, -23,
    -24, 24, -25, 25, -26, 26, -27, 27, -28, 28, -29, 29, -30, 30, -31, 31,
    -32, 32, -33, 33, -34, 34, -35, 35, -36, 36, -37, 37, -38, 38, -39, 39,
    -40, 40, -41, 41, -42, 42, -43, 43, -44, 44, -45, 45, -46, 46, -47, 47,
    -48, 48, -49, 49, -50, 50, -51, 51, -52, 52, -53, 53, -54, 54, -55, 55,
    -56, 56, -57, 57, -58, 58, -59, 59, -60, 60, -61, 61, -62, 62, -63, 63,
    -64, 64, -65, 65, -66, 66, -67, 67, -68, 68, -69, 69, -70, 70, -71, 71,
    -72, 72, -73, 73, -74, 74, -75, 75, -76, 76, -77, 77, -78, 78, -79, 79,
    -80, 80, -81, 81, -82, 82, -83, 83, -84, 84, -85, 85, -86, 86, -87, 87,
    -88, 88, -89, 89, -90, 90, -91, 91, -92, 92, -93, 93, -94, 94, -95, 95,
    -96, 96, -97, 97, -98, 98, -99, 99, -100, 100, -101, 101, -102, 102, -103, 103,
    -104, 104, -105, 105, -106, 106, -107, 107, -108, 108, -109, 109, -110, 110, -111, 111,
    -112, 112, -113, 113, -114, 114, -115, 115, -116, 116, -117, 117, -118, 118, -119, 119,
    -120, 120, -121, 121, -122, 122, -123, 123, -124, 124, -125, 125, -126, 126, -127, 127
};

int AesOfb::rcon[11] = {
    0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36
};

// ---------------------------------------------------------------------
//  Demo driver (generates random data, encrypts, prints hex)
// ---------------------------------------------------------------------
int main() {
    std::srand(0);                     // deterministic “random”

    // generate random 16‑byte key
    std::vector<int> keyBytes(16);
    for (int i = 0; i < 16; ++i) keyBytes[i] = std::rand() & 0xff;

    // generate random plaintext (multiple of 16, here 64 bytes)
    int plainLen = 64;
    std::vector<int> plain(plainLen);
    for (int i = 0; i < plainLen; ++i) plain[i] = std::rand() & 0xff;

    // introduce explicit zero and positive extremes in plaintext
    plain[0] = 0;
    plain[5] = 0;
    plain[10] = 127;
    plain[15] = 255;

    // AES‑OFB encryption
    AesOfb cipher(keyBytes);
    std::vector<int> cipherText = cipher.encrypt(plain);

    // output key, plaintext and ciphertext in hex
    std::cout << "Key:        ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << keyBytes[i];
    std::cout << "\nPlaintext:  ";
    for (int i = 0; i < plainLen; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i];
    std::cout << "\nCiphertext: ";
    for (int i = 0; i < (int)cipherText.size(); ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << cipherText[i];
    std::cout << std::dec << std::endl;
    return 0;
}
