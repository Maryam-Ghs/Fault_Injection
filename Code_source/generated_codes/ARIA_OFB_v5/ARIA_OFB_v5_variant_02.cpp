#include <iostream>
#include <iomanip>

//////////////////////////////////////////////////////////////////////
// Helper: rotate left 8‑bit value stored in an int (0‑255 range)
int rotl8(int v, int n) {
    return ((v << n) & 0xFF) | (v >> (8 - n));
}

// Helper: rotate right 8‑bit value stored in an int (0‑255 range)
int rotr8(int v, int n) {
    return ((v >> n) & 0xFF) | ((v << (8 - n)) & 0xFF);
}

// --------------------------------------------------------------------
void expand_key(int *user_key, int key_len, int **round_keys, int *round_cnt) {
    if (key_len == 16) *round_cnt = 12;
    else if (key_len == 24) *round_cnt = 14;
    else *round_cnt = 16;
    int total_bytes = (*round_cnt) * 16;
    *round_keys = new int[total_bytes];
    int i = 0;
    while (i < total_bytes) {
        int r = i / 16;
        int j = i % 16;
        int src = j % key_len;
        (*round_keys)[i] = (user_key[src] + r) & 0xFF;
        i = i + 1;
    }
}

// --------------------------------------------------------------------
void encrypt_block(int *rk, int rounds, int *src, int *dst) {
    int state[16];
    int idx = 0;
    while (idx < 16) {
        state[idx] = src[idx];
        idx = idx + 1;
    }
    int r = 0;
    while (r < rounds) {
        int j = 0;
        while (j < 16) {
            state[j] = state[j] ^ rk[r * 16 + j];
            j = j + 1;
        }
        int rot = r % 8;
        int k = 0;
        while (k < 16) {
            state[k] = rotl8(state[k], rot);
            k = k + 1;
        }
        int m = 0;
        while (m < 16) {
            int hi = (state[m] >> 4) & 0x0F;
            int lo = state[m] & 0x0F;
            state[m] = (lo << 4) | hi;
            m = m + 1;
        }
        r = r + 1;
    }
    int p = 0;
    while (p < 16) {
        dst[p] = state[p];
        p = p + 1;
    }
}

// --------------------------------------------------------------------
void ofb_encrypt(int *user_key, int key_len,
                 int *iv, int *pt, int pt_len,
                 int *ct) {
    int *rk = 0;
    int round_cnt = 0;
    expand_key(user_key, key_len, &rk, &round_cnt);
    int *feedback = new int[16];
    int i = 0;
    while (i < 16) {
        feedback[i] = iv[i];
        i = i + 1;
    }
    int *stream = new int[16];
    int pos = 0;
    int processed = 0;
    while (processed < pt_len) {
        if (pos == 0) {
            encrypt_block(rk, round_cnt, feedback, stream);
            int t = 0;
            while (t < 16) {
                feedback[t] = stream[t];
                t = t + 1;
            }
        }
        ct[processed] = pt[processed] ^ stream[pos];
        processed = processed + 1;
        pos = (pos + 1) % 16;
    }
    delete[] feedback;
    delete[] stream;
    delete[] rk;
}

// --------------------------------------------------------------------
void dump_hex(const char *label, int *buf, int len) {
    std::cout << label;
    int i = 0;
    while (i < len) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (buf[i] & 0xFF);
        if ((i + 1) % 16 == 0) std::cout << ' ';
        i = i + 1;
    }
    std::cout << std::dec << std::endl;
}

/* LLM input variant 2: small-diverse */
int main() {
    // 1) Zero‑length plaintext
    int pt0_len = 0;
    int *pt0 = new int[pt0_len];

    // 2) One‑byte plaintext (value 0x3C)
    int pt1_len = 1;
    int *pt1 = new int[pt1_len];
    pt1[0] = 0x3C;

    // 3) Small 5‑byte plaintext
    int pt2_len = 5;
    int *pt2 = new int[pt2_len];
    for (int i = 0; i < pt2_len; ++i) pt2[i] = (i * 13 + 7) & 0xFF;

    // 4) Exactly one block (16 bytes) with varied pattern
    int pt3_len = 16;
    int *pt3 = new int[pt3_len];
    for (int i = 0; i < pt3_len; ++i) pt3[i] = (i * 17) ^ 0xAA;

    // 5) Non‑multiple length (21 bytes)
    int pt4_len = 21;
    int *pt4 = new int[pt4_len];
    for (int i = 0; i < pt4_len; ++i) pt4[i] = (i * 23 + 5) & 0xFF;

    // 6) Medium‑size (48 bytes) plaintext
    int pt5_len = 48;
    int *pt5 = new int[pt5_len];
    for (int i = 0; i < pt5_len; ++i) pt5[i] = ((i * 31) ^ 0x55) & 0xFF;

    // 24‑byte key (medium size) and 16‑byte IV
    int key_len = 24;
    int *key = new int[key_len];
    for (int i = 0; i < key_len; ++i) key[i] = (i * 9 + 3) & 0xFF;

    int *iv = new int[16];
    for (int i = 0; i < 16; ++i) iv[i] = (i * 5 + 1) & 0xFF;

    auto run_case = [&](const char *name, int *plain, int plen) {
        int *cipher = new int[plen];
        ofb_encrypt(key, key_len, iv, plain, plen, cipher);
        std::cout << "=== " << name << " ===" << std::endl;
        dump_hex("Plain : ", plain, plen);
        dump_hex("Cipher: ", cipher, plen);
        std::cout << std::endl;
        delete[] cipher;
    };

    run_case("Zero‑length", pt0, pt0_len);
    run_case("One‑byte", pt1, pt1_len);
    run_case("Five‑bytes", pt2, pt2_len);
    run_case("One‑block", pt3, pt3_len);
    run_case("Non‑multiple (21)", pt4, pt4_len);
    run_case("Medium (48)", pt5, pt5_len);

    delete[] pt0; delete[] pt1; delete[] pt2; delete[] pt3; delete[] pt4; delete[] pt5;
    delete[] key; delete[] iv;
    return 0;
}
