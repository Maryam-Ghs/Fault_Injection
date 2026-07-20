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
// Generate a very simple ARIA‑like key schedule.
// For demonstration we expand the user key into (rounds * 16) bytes
// using a deterministic byte‑wise transformation.
void expand_key(int *user_key, int key_len, int **round_keys, int *round_cnt) {
    // ARIA uses 12, 14 or 16 rounds for 128/192/256‑bit keys.
    // Here we map: 16‑byte key → 12 rounds, 24‑byte → 14, 32‑byte → 16.
    if (key_len == 16) *round_cnt = 12;
    else if (key_len == 24) *round_cnt = 14;
    else *round_cnt = 16;               // fallback for 32‑byte key

    int total_bytes = (*round_cnt) * 16;
    *round_keys = new int[total_bytes];

    // Simple diffusion: each round key byte = (key byte + round index) mod 256
    int i = 0;
    while (i < total_bytes) {
        int r = i / 16;                 // current round
        int j = i % 16;                 // byte inside round
        int src = j % key_len;
        (*round_keys)[i] = (user_key[src] + r) & 0xFF;
        i = i + 1;
    }
}

// --------------------------------------------------------------------
// Encrypt a single 16‑byte block using the generated round keys.
// The algorithm mirrors ARIA’s structure but is heavily simplified.
void encrypt_block(int *rk, int rounds, int *src, int *dst) {
    // Working buffer, start with a copy of the plaintext.
    int state[16];
    int idx = 0;
    while (idx < 16) {
        state[idx] = src[idx];
        idx = idx + 1;
    }

    // Each round consists of: XOR with round key → byte rotation → substitution.
    int r = 0;
    while (r < rounds) {
        // ----- XOR with round key -----
        int j = 0;
        while (j < 16) {
            state[j] = state[j] ^ rk[r * 16 + j];
            j = j + 1;
        }

        // ----- Rotate every byte left by (r mod 8) positions -----
        int rot = r % 8;
        int k = 0;
        while (k < 16) {
            state[k] = rotl8(state[k], rot);
            k = k + 1;
        }

        // ----- Simple substitution: swap high/low nibble -----
        int m = 0;
        while (m < 16) {
            int hi = (state[m] >> 4) & 0x0F;
            int lo = state[m] & 0x0F;
            state[m] = (lo << 4) | hi;
            m = m + 1;
        }

        r = r + 1;
    }

    // Write the ciphertext out.
    int p = 0;
    while (p < 16) {
        dst[p] = state[p];
        p = p + 1;
    }
}

// --------------------------------------------------------------------
// OFB mode encryption using the above block cipher.
// iv is a 16‑byte initialization vector.
// pt_len may be any non‑negative value (including zero).
void ofb_encrypt(int *user_key, int key_len,
                 int *iv, int *pt, int pt_len,
                 int *ct) {

    int *rk = 0;
    int round_cnt = 0;
    expand_key(user_key, key_len, &rk, &round_cnt);

    // Current feedback block (starts with IV)
    int *feedback = new int[16];
    int i = 0;
    while (i < 16) {
        feedback[i] = iv[i];
        i = i + 1;
    }

    // Buffer for the generated keystream block
    int *stream = new int[16];

    int pos = 0;                // position inside current keystream block
    int processed = 0;          // total bytes processed

    while (processed < pt_len) {
        // Re‑fill keystream when we have consumed the previous one
        if (pos == 0) {
            encrypt_block(rk, round_cnt, feedback, stream);
            // In OFB the next feedback is the keystream itself
            int t = 0;
            while (t < 16) {
                feedback[t] = stream[t];
                t = t + 1;
            }
        }

        // XOR plaintext byte with keystream byte
        ct[processed] = pt[processed] ^ stream[pos];

        // Advance counters
        processed = processed + 1;
        pos = (pos + 1) % 16;
    }

    delete[] feedback;
    delete[] stream;
    delete[] rk;
}

// --------------------------------------------------------------------
// Utility: print a byte array as hexadecimal values.
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

// --------------------------------------------------------------------
/* LLM input variant 1: minimal-boundary */
int main() {
    // 1) Zero‑length plaintext
    int pt0_len = 0;
    int *pt0 = new int[pt0_len];               // empty

    // 2) One‑byte plaintext
    int pt1_len = 1;
    int *pt1 = new int[pt1_len];
    pt1[0] = 0x01;

    // 3) Exactly one block (16 bytes) – minimal pattern
    int pt2_len = 16;
    int *pt2 = new int[pt2_len];
    int idx = 0;
    while (idx < pt2_len) {
        pt2[idx] = idx;                         // 0..15
        idx = idx + 1;
    }

    // 4) Non‑multiple length (17 bytes) – smallest non‑multiple > block
    int pt3_len = 17;
    int *pt3 = new int[pt3_len];
    idx = 0;
    while (idx < pt3_len) {
        pt3[idx] = (idx * 5) & 0xFF;
        idx = idx + 1;
    }

    // 5) Larger data (32 bytes) – minimal larger size
    int pt4_len = 32;
    int *pt4 = new int[pt4_len];
    idx = 0;
    while (idx < pt4_len) {
        pt4[idx] = (idx * 3) ^ 0x0F;
        idx = idx + 1;
    }

    // Minimal key (16‑byte) and IV (16‑byte)
    int key_len = 16;
    int *key = new int[key_len];
    idx = 0;
    while (idx < key_len) {
        key[idx] = (idx * 7) & 0xFF;
        idx = idx + 1;
    }

    int *iv = new int[16];
    idx = 0;
    while (idx < 16) {
        iv[idx] = (idx * 11) & 0xFF;
        idx = idx + 1;
    }

    // Helper lambda to process a single case
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
    run_case("One‑byte",    pt1, pt1_len);
    run_case("One‑block",   pt2, pt2_len);
    run_case("Non‑multiple", pt3, pt3_len);
    run_case("Larger (32)",  pt4, pt4_len);

    // Clean up allocated memory
    delete[] pt0;
    delete[] pt1;
    delete[] pt2;
    delete[] pt3;
    delete[] pt4;
    delete[] key;
    delete[] iv;

    return 0;
}
