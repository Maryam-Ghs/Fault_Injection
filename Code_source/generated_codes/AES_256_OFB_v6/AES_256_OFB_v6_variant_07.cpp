/* LLM input variant 7: reverse-adversarial */
// AES-256-OFB implementation – version #6
// All code resides inside main(), uses only int and std::vector, and follows a loop‑heavy style.

#include <iostream>
#include <vector>
#include <iomanip>

int main() {
    // ---------- S‑Box ----------
    std::vector<int> sbox(256);
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
    for (int i = 0; i < 256; ++i) sbox[i] = sb[i];

    // ---------- Rcon ----------
    std::vector<int> rcon(15);
    int rc[15] = {1,2,4,8,16,32,64,128,27,54,108,216,171,77,154};
    for (int i = 0; i < 15; ++i) rcon[i] = rc[i];

    // ---------- Test vectors ----------
    // 256‑bit key: 0x1F,0x1E,...,0x00 (reverse order)
    std::vector<int> key(32);
    for (int i = 0; i < 32; ++i) key[i] = 31 - i;

    // 128‑bit IV: 0x0F,0x0E,...,0x00 (reverse order)
    std::vector<int> iv(16);
    for (int i = 0; i < 16; ++i) iv[i] = 15 - i;

    // Plaintext: four blocks, descending bytes 0x3F..0x00
    std::vector<int> plain(64);
    for (int i = 0; i < 64; ++i) plain[i] = 63 - i;

    // ---------- Helper lambdas ----------
    auto xtime = [&](int x) -> int {
        int m = (x << 1) & 0xFF;
        if (x & 0x80) m ^= 0x1B;
        return m;
    };

    // ---------- Key expansion ----------
    const int Nb = 4;      // block size in words
    const int Nk = 8;      // key length in words (256‑bit)
    const int Nr = 14;     // rounds
    std::vector<int> roundKey(16 * (Nr + 1)); // 240 bytes

    // copy initial key
    for (int i = 0; i < 32; ++i) roundKey[i] = key[i];

    // expand
    int iWord = Nk;
    while (iWord < Nb * (Nr + 1)) {
        int temp[4];
        // copy previous word
        for (int b = 0; b < 4; ++b) temp[b] = roundKey[(iWord - 1) * 4 + b];

        if (iWord % Nk == 0) {
            // RotWord
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // SubWord
            for (int b = 0; b < 4; ++b) temp[b] = sbox[temp[b]];
            // Rcon
            temp[0] ^= rcon[iWord / Nk - 1];
        } else if (Nk > 6 && (iWord % Nk) == 4) {
            // SubWord only
            for (int b = 0; b < 4; ++b) temp[b] = sbox[temp[b]];
        }

        // XOR with word Nk positions back
        for (int b = 0; b < 4; ++b) {
            roundKey[iWord * 4 + b] = roundKey[(iWord - Nk) * 4 + b] ^ temp[b];
        }
        ++iWord;
    }

    // ---------- Core AES operations ----------
    auto addRoundKey = [&](std::vector<int>& st, int round) {
        for (int i = 0; i < 16; ++i) {
            st[i] ^= roundKey[round * 16 + i];
        }
    };

    auto subBytes = [&](std::vector<int>& st) {
        for (int i = 0; i < 16; ++i) {
            st[i] = sbox[st[i]];
        }
    };

    auto shiftRows = [&](std::vector<int>& st) {
        int tmp;
        // Row 1 shift left by 1
        tmp = st[1];
        st[1]  = st[5];
        st[5]  = st[9];
        st[9]  = st[13];
        st[13] = tmp;
        // Row 2 shift left by 2
        tmp = st[2];
        st[2]  = st[10];
        st[10] = tmp;
        tmp = st[6];
        st[6]  = st[14];
        st[14] = tmp;
        // Row 3 shift left by 3 (right by 1)
        tmp = st[15];
        st[15] = st[11];
        st[11] = st[7];
        st[7]  = st[3];
        st[3]  = tmp;
    };

    auto mixColumns = [&](std::vector<int>& st) {
        for (int c = 0; c < 4; ++c) {
            int i0 = c * 4 + 0;
            int i1 = c * 4 + 1;
            int i2 = c * 4 + 2;
            int i3 = c * 4 + 3;

            int a0 = st[i0];
            int a1 = st[i1];
            int a2 = st[i2];
            int a3 = st[i3];

            int m0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            int m1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            int m2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            int m3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);

            st[i0] = m0 & 0xFF;
            st[i1] = m1 & 0xFF;
            st[i2] = m2 & 0xFF;
            st[i3] = m3 & 0xFF;
        }
    };

    // ---------- Block encryption ----------
    auto encryptBlock = [&](const std::vector<int>& inBlock) -> std::vector<int> {
        std::vector<int> st(16);
        for (int i = 0; i < 16; ++i) st[i] = inBlock[i];

        addRoundKey(st, 0);
        for (int r = 1; r < Nr; ++r) {
            subBytes(st);
            shiftRows(st);
            mixColumns(st);
            addRoundKey(st, r);
        }
        subBytes(st);
        shiftRows(st);
        addRoundKey(st, Nr);

        return st;
    };

    // ---------- OFB mode ----------
    std::vector<int> previous = iv;               // IV as first input to cipher
    std::vector<int> ciphertext(plain.size());

    for (int blk = 0; blk < (int)plain.size() / 16; ++blk) {
        // generate keystream block
        std::vector<int> keystream = encryptBlock(previous);
        // XOR with plaintext
        for (int i = 0; i < 16; ++i) {
            int pt = plain[blk * 16 + i];
            ciphertext[blk * 16 + i] = pt ^ keystream[i];
        }
        // next input is the keystream itself (OFB)
        previous = keystream;
    }

    // ---------- Output ----------
    std::cout << "Ciphertext (hex):" << std::endl;
    for (size_t i = 0; i < ciphertext.size(); ++i) {
        if (i % 16 == 0) std::cout << std::endl;
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (ciphertext[i] & 0xFF) << " ";
    }
    std::cout << std::dec << std::endl;
    return 0;
}
