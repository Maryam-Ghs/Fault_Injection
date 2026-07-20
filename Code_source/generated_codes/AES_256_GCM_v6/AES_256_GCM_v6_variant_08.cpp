#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

//--------------------  Helper Tables  --------------------
std::vector<int> sbox = {
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

std::vector<int> rcon = {
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    27,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    54,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

//--------------------  Core Operations  --------------------
int xtime(int v) {
    int hi = v & 0x80;
    v = (v << 1) & 0xFF;
    if (hi) v ^= 0x1b;
    return v;
}

void sub_bytes(std::vector<int>& state) {
    int i = 0;
    while (i < 16) {
        state[i] = sbox[state[i]];
        ++i;
    }
}

void shift_rows(std::vector<int>& st) {
    // Row 1
    int tmp = st[1];
    st[1] = st[5];
    st[5] = st[9];
    st[9] = st[13];
    st[13] = tmp;
    // Row 2
    tmp = st[2];
    int tmp2 = st[6];
    st[2] = st[10];
    st[6] = st[14];
    st[10] = tmp;
    st[14] = tmp2;
    // Row 3
    tmp = st[3];
    st[3] = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7] = tmp;
}

void mix_columns(std::vector<int>& st) {
    int i = 0;
    while (i < 16) {
        int a0 = st[i];
        int a1 = st[i+1];
        int a2 = st[i+2];
        int a3 = st[i+3];
        int r0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        int r1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        int r2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        int r3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        st[i]   = r0 & 0xFF;
        st[i+1] = r1 & 0xFF;
        st[i+2] = r2 & 0xFF;
        st[i+3] = r3 & 0xFF;
        i += 4;
    }
}

void add_round_key(std::vector<int>& st, const std::vector<int>& rk) {
    int i = 0;
    while (i < 16) {
        st[i] ^= rk[i];
        ++i;
    }
}

//--------------------  Key Schedule  --------------------
std::vector< std::vector<int> > expand_key(const std::vector<int>& key) {
    // key = 32 bytes -> 8 words (4 bytes each)
    std::vector< std::vector<int> > round_keys;
    int i = 0;
    while (i < 8) {
        std::vector<int> w(4);
        for (int j = 0; j < 4; ++j) w[j] = key[4*i + j];
        round_keys.push_back(w);
        ++i;
    }
    int total_words = 4 * (14 + 1); // 4 words per round key
    while ((int)round_keys.size() < total_words) {
        std::vector<int> temp = round_keys.back();
        if (((int)round_keys.size() % 8) == 0) {
            // RotWord
            int t = temp[0];
            for (int j = 0; j < 3; ++j) temp[j] = temp[j+1];
            temp[3] = t;
            // SubWord
            for (int j = 0; j < 4; ++j) temp[j] = sbox[temp[j]];
            // Rcon
            int rc = rcon[4*((int)round_keys.size()/8)];
            temp[0] ^= rc;
        } else if (((int)round_keys.size() % 8) == 4) {
            for (int j = 0; j < 4; ++j) temp[j] = sbox[temp[j]];
        }
        // XOR with word 8 positions earlier
        std::vector<int> prev = round_keys[round_keys.size() - 8];
        std::vector<int> neww(4);
        for (int j = 0; j < 4; ++j) neww[j] = prev[j] ^ temp[j];
        round_keys.push_back(neww);
    }
    // Convert to round key blocks (16 bytes each)
    std::vector< std::vector<int> > blocks;
    int r = 0;
    while (r < (int)round_keys.size()) {
        std::vector<int> block(16);
        for (int c = 0; c < 4; ++c) {
            for (int b = 0; b < 4; ++b) {
                block[4*c + b] = round_keys[r + c][b];
            }
        }
        blocks.push_back(block);
        r += 4;
    }
    return blocks;
}

//--------------------  AES Block Encrypt  --------------------
std::vector<int> aes_encrypt_block(const std::vector<int>& plain,
                                   const std::vector< std::vector<int> >& rkeys) {
    std::vector<int> state = plain;
    add_round_key(state, rkeys[0]);
    int round = 1;
    while (round < 14) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, rkeys[round]);
        ++round;
    }
    // Final round (no MixColumns)
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, rkeys[14]);
    return state;
}

//--------------------  GCM Supporting Functions  --------------------
void inc_counter(std::vector<int>& ctr) {
    int i = 15;
    while (i >= 0) {
        ctr[i] = (ctr[i] + 1) & 0xFF;
        if (ctr[i] != 0) break;
        --i;
    }
}

std::vector<int> shift_right_one(std::vector<int> v) {
    int lsb = v[15] & 1;
    int carry = 0;
    int i = 15;
    while (i >= 0) {
        int newc = v[i] & 1;
        v[i] = (v[i] >> 1) | (carry << 7);
        carry = newc;
        --i;
    }
    if (lsb) v[0] ^= 0xE1;
    return v;
}

std::vector<int> gf_mul(std::vector<int> X, const std::vector<int>& Y) {
    std::vector<int> Z(16,0);
    int i = 0;
    while (i < 128) {
        int byteIdx = i / 8;
        int bitIdx  = 7 - (i % 8);
        if ((Y[byteIdx] >> bitIdx) & 1) {
            int j = 0;
            while (j < 16) { Z[j] ^= X[j]; ++j; }
        }
        X = shift_right_one(X);
        ++i;
    }
    return Z;
}

std::vector<int> ghash(const std::vector<int>& H, const std::vector<int>& data) {
    std::vector<int> Y(16,0);
    int total = (int)data.size();
    int blocks = (total + 15) / 16;
    int b = 0;
    while (b < blocks) {
        std::vector<int> blk(16,0);
        int j = 0;
        while (j < 16) {
            int idx = b*16 + j;
            if (idx < total) blk[j] = data[idx];
            ++j;
        }
        int k = 0;
        while (k < 16) { Y[k] ^= blk[k]; ++k; }
        Y = gf_mul(Y, H);
        ++b;
    }
    return Y;
}

std::vector<int> to_be64(int value) {
    std::vector<int> out(8,0);
    for (int i = 7; i >= 0; --i) {
        out[i] = value & 0xFF;
        value >>= 8;
    }
    return out;
}

//--------------------  GCM Encryption  --------------------
void gcm_encrypt(const std::vector<int>& key,
                 const std::vector<int>& iv,
                 const std::vector<int>& plain,
                 const std::vector<int>& aad,
                 std::vector<int>& cipher,
                 std::vector<int>& tag) {
    // ---------- Key schedule ----------
    std::vector< std::vector<int> > round_keys = expand_key(key);

    // ---------- Compute H ----------
    std::vector<int> zeroBlock(16,0);
    std::vector<int> H = aes_encrypt_block(zeroBlock, round_keys);

    // ---------- Prepare J0 ----------
    std::vector<int> J0 = iv;
    J0.resize(12);
    J0.push_back(0);
    J0.push_back(0);
    J0.push_back(0);
    J0.push_back(1);

    // ---------- Encryption (CTR) ----------
    std::vector<int> ctr = J0;
    int ptLen = (int)plain.size();
    int blkCnt = (ptLen + 15) / 16;
    int i = 0;
    while (i < blkCnt) {
        inc_counter(ctr);
        std::vector<int> ks = aes_encrypt_block(ctr, round_keys);
        std::vector<int> ptBlk(16,0);
        int j = 0;
        while (j < 16) {
            int idx = i*16 + j;
            if (idx < ptLen) ptBlk[j] = plain[idx];
            ++j;
        }
        std::vector<int> ctBlk(16,0);
        int k = 0;
        while (k < 16) { ctBlk[k] = ptBlk[k] ^ ks[k]; ++k; }
        int jdx = i*16;
        int l = 0;
        while (l < 16 && jdx + l < ptLen) {
            cipher.push_back(ctBlk[l]);
            ++l;
        }
        ++i;
    }

    // ---------- GHASH ----------
    std::vector<int> authData = aad;
    // Pad AAD to 16‑byte boundary
    while ((int)authData.size() % 16) authData.push_back(0);
    // Pad ciphertext to 16‑byte boundary
    std::vector<int> ctPadded = cipher;
    while ((int)ctPadded.size() % 16) ctPadded.push_back(0);
    // Length block
    std::vector<int> lenBlock(16,0);
    std::vector<int> aadBits = to_be64((int)aad.size()*8);
    std::vector<int> ctBits  = to_be64((int)cipher.size()*8);
    for (int m = 0; m < 8; ++m) lenBlock[m]   = aadBits[m];
    for (int m = 0; m < 8; ++m) lenBlock[8+m] = ctBits[m];

    // Concatenate all pieces for GHASH
    std::vector<int> ghashInput;
    ghashInput.insert(ghashInput.end(), authData.begin(), authData.end());
    ghashInput.insert(ghashInput.end(), ctPadded.begin(), ctPadded.end());
    ghashInput.insert(ghashInput.end(), lenBlock.begin(), lenBlock.end());

    std::vector<int> S = ghash(H, ghashInput);

    // ---------- Tag ----------
    std::vector<int> eJ0 = aes_encrypt_block(J0, round_keys);
    tag.resize(16);
    int t = 0;
    while (t < 16) { tag[t] = eJ0[t] ^ S[t]; ++t; }
}

//--------------------  Utility --------------------
void print_hex(const std::vector<int>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (data[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;
}

//--------------------  Main  --------------------
int main() {
    // 256‑bit key (mostly zeros, a few non‑zero bytes)
    std::vector<int> key(32, 0);
    key[0] = 0x00;
    key[1] = 0xFF;
    key[15] = 0xAA;
    key[31] = 0x55;

    // 12‑byte IV (standard sequential)
    std::vector<int> iv = {
        0x00,0x01,0x02,0x03,0x04,0x05,
        0x06,0x07,0x08,0x09,0x0a,0x0b
    };

    // Plaintext: 256 bytes, mostly zeros with sparse non‑zero values
    std::vector<int> plain(256, 0);
    plain[0]  = 'H';
    plain[50] = 'i';
    plain[123] = '!';
    plain[200] = 0x7F;
    plain[255] = 'Z';

    // Additional authenticated data (empty for sparsity)
    std::vector<int> aad; // no AAD

    std::vector<int> cipher;
    std::vector<int> tag;

    gcm_encrypt(key, iv, plain, aad, cipher, tag);

    std::cout << "Ciphertext: ";
    print_hex(cipher);
    std::cout << "Tag:        ";
    print_hex(tag);
    return 0;
}
