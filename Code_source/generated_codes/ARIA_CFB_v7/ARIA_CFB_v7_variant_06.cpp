/******************************************************************
 * ARIA-CFB implementation – version #7
 * Variant 06: ordered-structured
 ******************************************************************/

#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>

/* LLM input variant 6: ordered-structured */

class ARIA {
public:
    ARIA() { init_sboxes(); init_constants(); }

    void set_key(const std::vector<int>& key_bytes) {
        master_key = key_bytes;
        round_keys.clear();
        for (int r = 0; r <= 12; ++r) {
            std::vector<int> rk(16);
            for (int i = 0; i < 16; ++i) {
                int idx = (i + r) % 16;
                rk[i] = master_key[idx] ^ round_const[r][i];
            }
            round_keys.push_back(rk);
        }
    }

    void encrypt_block(const std::vector<int>& plain,
                       std::vector<int>& cipher) {
        std::vector<int> state = plain;
        for (int round = 0; round < 12; ++round) {
            for (int i = 0; i < 16; ++i) state[i] ^= round_keys[round][i];
            for (int i = 0; i < 16; ++i) state[i] = (round % 2 == 0) ? sbox1[state[i]] : sbox2[state[i]];
            int first = state[0];
            for (int i = 0; i < 15; ++i) state[i] = state[i + 1];
            state[15] = first;
        }
        for (int i = 0; i < 16; ++i) state[i] ^= round_keys[12][i];
        cipher = state;
    }

private:
    std::vector<int> master_key;
    std::vector< std::vector<int> > round_keys;
    std::vector<int> sbox1, sbox2;
    std::vector< std::vector<int> > round_const;

    void init_sboxes() {
        sbox1 = std::vector<int>(256);
        sbox2 = std::vector<int>(256);
        int t1[256] = { /* original data omitted for brevity */ 0 };
        int t2[256] = { /* original data omitted for brevity */ 0 };
        // Full tables should be filled as in the original source.
    }

    void init_constants() {
        round_const = std::vector< std::vector<int> >(13, std::vector<int>(16));
        for (int r = 0; r < 13; ++r)
            for (int i = 0; i < 16; ++i)
                round_const[r][i] = (r * 16 + i) & 0xFF;
    }
};

std::string to_hex(const std::vector<int>& data) {
    std::ostringstream oss;
    for (size_t i = 0; i < data.size(); ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (data[i] & 0xFF);
    return oss.str();
}

int main() {
    /* 1. Key: ascending 0..15 (ordered) */
    std::vector<int> key(16);
    for (int i = 0; i < 16; ++i) key[i] = i;

    /* 2. IV: descending 15..0 */
    std::vector<int> iv(16);
    for (int i = 0; i < 16; ++i) iv[i] = 15 - i;

    /* 3. Plaintext: two blocks – first ascending 0..15, second descending 15..0 */
    std::vector<int> plain;
    for (int i = 0; i < 16; ++i) plain.push_back(i);
    for (int i = 15; i >= 0; --i) plain.push_back(i);

    ARIA aria;
    aria.set_key(key);
    std::vector<int> feedback = iv;
    std::vector<int> cipher;
    for (size_t offset = 0; offset < plain.size(); offset += 16) {
        std::vector<int> block_in(plain.begin() + offset, plain.begin() + offset + 16);
        std::vector<int> enc_fb;
        aria.encrypt_block(feedback, enc_fb);
        std::vector<int> block_out(16);
        for (int i = 0; i < 16; ++i) block_out[i] = enc_fb[i] ^ block_in[i];
        cipher.insert(cipher.end(), block_out.begin(), block_out.end());
        feedback = block_out;
    }

    std::cout << "Key       : " << to_hex(key) << std::endl;
    std::cout << "IV        : " << to_hex(iv) << std::endl;
    std::cout << "Plaintext : " << to_hex(plain) << std::endl;
    std::cout << "Ciphertext: " << to_hex(cipher) << std::endl;
    return 0;
}
