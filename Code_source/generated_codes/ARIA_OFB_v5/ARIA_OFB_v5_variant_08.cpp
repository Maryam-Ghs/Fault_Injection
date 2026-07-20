#include <iostream>
#include <iomanip>

//////////////////////////////////////////////////////////////////////
int rotl8(int v, int n) { return ((v << n) & 0xFF) | (v >> (8 - n)); }
int rotr8(int v, int n) { return ((v >> n) & 0xFF) | ((v << (8 - n)) & 0xFF); }

void expand_key(int *user_key, int key_len, int **round_keys, int *round_cnt) {
    if (key_len == 16) *round_cnt = 12;
    else if (key_len == 24) *round_cnt = 14;
    else *round_cnt = 16;
    int total = (*round_cnt) * 16;
    *round_keys = new int[total];
    for (int i = 0; i < total; ++i) {
        int r = i / 16;
        int j = i % 16;
        int src = j % key_len;
        (*round_keys)[i] = (user_key[src] + r) & 0xFF;
    }
}

void encrypt_block(int *rk, int rounds, int *src, int *dst) {
    int state[16];
    for (int i = 0; i < 16; ++i) state[i] = src[i];
    for (int r = 0; r < rounds; ++r) {
        for (int j = 0; j < 16; ++j) state[j] ^= rk[r * 16 + j];
        int rot = r % 8;
        for (int k = 0; k < 16; ++k) state[k] = rotl8(state[k], rot);
        for (int m = 0; m < 16; ++m) {
            int hi = (state[m] >> 4) & 0x0F;
            int lo = state[m] & 0x0F;
            state[m] = (lo << 4) | hi;
        }
    }
    for (int i = 0; i < 16; ++i) dst[i] = state[i];
}

void ofb_encrypt(int *user_key, int key_len,
                 int *iv, int *pt, int pt_len,
                 int *ct) {
    int *rk = nullptr; int rounds = 0;
    expand_key(user_key, key_len, &rk, &rounds);
    int *feedback = new int[16];
    for (int i = 0; i < 16; ++i) feedback[i] = iv[i];
    int *stream = new int[16];
    int pos = 0, processed = 0;
    while (processed < pt_len) {
        if (pos == 0) {
            encrypt_block(rk, rounds, feedback, stream);
            for (int t = 0; t < 16; ++t) feedback[t] = stream[t];
        }
        ct[processed] = pt[processed] ^ stream[pos];
        ++processed; pos = (pos + 1) % 16;
    }
    delete[] feedback; delete[] stream; delete[] rk;
}

void dump_hex(const char *label, int *buf, int len) {
    std::cout << label;
    for (int i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (buf[i] & 0xFF);
        if ((i + 1) % 16 == 0) std::cout << ' ';
    }
    std::cout << std::dec << std::endl;
}

/* LLM input variant 8: sparse-skewed */
int main() {
    // Zero‑length
    int pt0_len = 0;
    int *pt0 = new int[pt0_len];

    // One‑byte (value 0x00)
    int pt1_len = 1;
    int *pt1 = new int[pt1_len];
    pt1[0] = 0x00;

    // One block – mostly zeros, few non‑zeros clustered in middle
    int pt2_len = 16;
    int *pt2 = new int[pt2_len];
    for (int i = 0; i < pt2_len; ++i) pt2[i] = 0x00;
    pt2[6] = 0xAA; pt2[7] = 0xBB; pt2[8] = 0xCC; pt2[9] = 0xDD;

    // Non‑multiple (20) – zeros with a cluster at positions 10‑14
    int pt3_len = 20;
    int *pt3 = new int[pt3_len];
    for (int i = 0; i < pt3_len; ++i) pt3[i] = 0x00;
    for (int i = 10; i <= 14; ++i) pt3[i] = 0x77 + (i - 10);

    // Larger (64) – sparse: every 8th byte is non‑zero
    int pt4_len = 64;
    int *pt4 = new int[pt4_len];
    for (int i = 0; i < pt4_len; ++i) pt4[i] = 0x00;
    for (int i = 0; i < pt4_len; i += 8) pt4[i] = 0x5A + (i / 8);

    // Key – mostly zeros, first 4 bytes non‑zero
    int key_len = 16;
    int *key = new int[key_len];
    for (int i = 0; i < key_len; ++i) key[i] = 0x00;
    key[0] = 0x11; key[1] = 0x22; key[2] = 0x33; key[3] = 0x44;

    // IV – all zeros
    int *iv = new int[16];
    for (int i = 0; i < 16; ++i) iv[i] = 0x00;

    auto run = [&](const char *name, int *plain, int plen) {
        int *cipher = new int[plen];
        ofb_encrypt(key, key_len, iv, plain, plen, cipher);
        std::cout << "=== " << name << " ===" << std::endl;
        dump_hex("Plain : ", plain, plen);
        dump_hex("Cipher: ", cipher, plen);
        std::cout << std::endl;
        delete[] cipher;
    };

    run("Zero‑length", pt0, pt0_len);
    run("One‑byte zero", pt1, pt1_len);
    run("Sparse block", pt2, pt2_len);
    run("Sparse non‑multiple", pt3, pt3_len);
    run("Sparse large", pt4, pt4_len);

    delete[] pt0; delete[] pt1; delete[] pt2; delete[] pt3; delete[] pt4;
    delete[] key; delete[] iv;
    return 0;
}
