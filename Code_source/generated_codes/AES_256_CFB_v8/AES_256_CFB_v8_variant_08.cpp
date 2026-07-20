#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

// ---------------------------------------------------------------------
//  AES-256 implementation (CFB mode) – version #8
//  All data types are int (no unsigned, long, double, const)
//  Helper functions are used, vector<int> for all buffers
// ---------------------------------------------------------------------

// -------------  S‑box  ------------------------------------------------
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

// -------------  Rcon  ------------------------------------------------
int rcon[15] = {
    1,2,4,8,16,32,64,128,27,54,108,216,171,205,123
};

// -------------  Helper: xtime for MixColumns  -----------------------
int xtime(int x) {
    return ((x << 1) ^ (((x >> 7) & 1) * 0x1b)) & 0xFF;
}

// -------------  SubBytes  --------------------------------------------
void sub_bytes(std::vector<int>& state) {
    for (int i = 0; i < 16; ++i) {
        state[i] = sbox[state[i]];
    }
}

// -------------  ShiftRows  -------------------------------------------
void shift_rows(std::vector<int>& st) {
    int tmp;

    // row 1 (shift 1)
    tmp = st[1]; st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = tmp;

    // row 2 (shift 2)
    tmp = st[2]; st[2] = st[10]; st[10] = tmp;
    tmp = st[6]; st[6] = st[14]; st[14] = tmp;

    // row 3 (shift 3)
    tmp = st[3]; st[3] = st[15]; st[15] = st[11]; st[11] = st[7]; st[7] = tmp;
}

// -------------  MixColumns  -----------------------------------------
void mix_columns(std::vector<int>& st) {
    for (int c = 0; c < 4; ++c) {
        int i0 = c * 4;
        int a0 = st[i0];
        int a1 = st[i0 + 1];
        int a2 = st[i0 + 2];
        int a3 = st[i0 + 3];

        int t = a0 ^ a1 ^ a2 ^ a3;
        int u = a0;

        st[i0]     ^= t ^ xtime(a0 ^ a1);
        st[i0 + 1] ^= t ^ xtime(a1 ^ a2);
        st[i0 + 2] ^= t ^ xtime(a2 ^ a3);
        st[i0 + 3] ^= t ^ xtime(a3 ^ u);
    }
}

// -------------  AddRoundKey  -----------------------------------------
void add_round_key(std::vector<int>& st, const std::vector<int>& rk, int round) {
    int off = round * 16;
    for (int i = 0; i < 16; ++i) {
        st[i] ^= rk[off + i];
    }
}

// -------------  Key Expansion (256‑bit)  -----------------------------
void expand_key(const std::vector<int>& key, std::vector<int>& roundKeys) {
    // key length = 32 bytes, words = 8
    for (int i = 0; i < 32; ++i) {
        roundKeys[i] = key[i];
    }

    int bytesGenerated = 32;
    int rconIdx = 0;
    std::vector<int> temp(4);

    while (bytesGenerated < 240) {
        // copy previous word
        for (int i = 0; i < 4; ++i) {
            temp[i] = roundKeys[bytesGenerated - 4 + i];
        }

        // every 32 bytes apply core
        if ((bytesGenerated % 32) == 0) {
            // RotWord
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // SubWord
            for (int i = 0; i < 4; ++i) {
                temp[i] = sbox[temp[i]];
            }
            // Rcon
            temp[0] ^= rcon[rconIdx];
            ++rconIdx;
        } else if ((bytesGenerated % 32) == 16) {
            // SubWord only
            for (int i = 0; i < 4; ++i) {
                temp[i] = sbox[temp[i]];
            }
        }

        // XOR with word 8 positions back
        for (int i = 0; i < 4; ++i) {
            roundKeys[bytesGenerated] = roundKeys[bytesGenerated - 32] ^ temp[i];
            ++bytesGenerated;
        }
    }
}

// -------------  AES encrypt single block  ----------------------------
void encrypt_block(const std::vector<int>& roundKeys, const std::vector<int>& in, std::vector<int>& out) {
    std::vector<int> state(16);
    for (int i = 0; i < 16; ++i) {
        state[i] = in[i];
    }

    add_round_key(state, roundKeys, 0);

    for (int round = 1; round <= 13; ++round) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, roundKeys, round);
    }

    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, roundKeys, 14);

    for (int i = 0; i < 16; ++i) {
        out[i] = state[i];
    }
}

// -------------  CFB mode (encrypt)  ---------------------------------
std::vector<int> cfb_encrypt(const std::vector<int>& key,
                             const std::vector<int>& iv,
                             const std::vector<int>& plain) {
    std::vector<int> roundKeys(240);
    expand_key(key, roundKeys);

    std::vector<int> prev = iv;               // feedback register
    std::vector<int> out(plain.size());

    int pos = 0;
    while (pos < (int)plain.size()) {
        std::vector<int> ks(16);
        encrypt_block(roundKeys, prev, ks);   // keystream

        for (int i = 0; i < 16 && pos + i < (int)plain.size(); ++i) {
            out[pos + i] = plain[pos + i] ^ ks[i];
        }

        // next feedback = ciphertext block (full 16 bytes)
        for (int i = 0; i < 16 && pos + i < (int)plain.size(); ++i) {
            prev[i] = out[pos + i];
        }

        pos += 16;
    }
    return out;
}

// -------------  Helper: print hex vector  ---------------------------
void dump_hex(const std::vector<int>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (data[i] & 0xFF);
        if ((i + 1) % 16 == 0) std::cout << "\n";
        else std::cout << " ";
    }
    std::cout << std::dec;
}

// -------------  Main – deterministic sparse test vector ------------
int main() {
    // 256‑bit key: sparse, mostly zeros
    std::vector<int> key(32, 0);
    key[0] = 0x1A;
    key[7] = 0x2B;
    key[15] = 0x3C;
    key[31] = 0x4D;

    // IV: sparse, mostly zeros
    std::vector<int> iv(16, 0);
    iv[0] = 0x55;
    iv[12] = 0xAA;

    // Plaintext: 32 bytes (two blocks), sparse with a few non‑zero bytes
    std::vector<int> plain(32, 0);
    plain[0] = 0x48;   // 'H'
    plain[1] = 0x69;   // 'i'
    plain[8] = 0x21;   // '!'
    plain[15] = 0xFF;
    plain[24] = 0x0F;
    plain[31] = 0xAA;

    std::vector<int> cipher = cfb_encrypt(key, iv, plain);

    std::cout << "Ciphertext (hex):\n";
    dump_hex(cipher);
    return 0;
}
