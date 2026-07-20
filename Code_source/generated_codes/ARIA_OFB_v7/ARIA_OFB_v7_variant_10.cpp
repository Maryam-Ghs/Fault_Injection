/* LLM input variant 10: large-safe-stress */
#include <iostream>
#include <vector>
#include <iomanip>

int main() {
    // version 7

    // ---------- helper: deterministic pseudo‑random generator ----------
    int seed = 555555555; // fixed seed
    auto next_rand = [&]() {
        seed = (seed * 1103515245 + 12345);
        return (seed >> 16) & 0x7fff;
    };

    // ---------- S‑boxes ----------
    int S1[256];
    int S2[256];
    int i = 0;
    while (i < 256) {
        S1[i] = i;
        S2[i] = 255 - i;
        i = i + 1;
    }

    // ---------- deterministic key ----------
    std::vector<int> key(16);
    i = 0;
    while (i < 16) {
        key[i] = (i * 13) & 0xff; // simple pattern
        i = i + 1;
    }

    // ---------- round keys ----------
    std::vector<std::vector<int>> roundKey(12, std::vector<int>(16));
    int r = 0;
    while (r < 12) {
        int j = 0;
        while (j < 16) {
            int idx = (j + r) & 15;
            roundKey[r][j] = key[idx];
            j = j + 1;
        }
        r = r + 1;
    }

    // ---------- deterministic IV ----------
    std::vector<int> iv(16);
    i = 0;
    while (i < 16) {
        iv[i] = (i * 7 + 3) & 0xff;
        i = i + 1;
    }

    // ---------- large plaintext (1024 bytes) ----------
    const int plain_len = 1024;
    std::vector<int> plain(plain_len);
    i = 0;
    while (i < plain_len) {
        plain[i] = next_rand() & 0xff;
        i = i + 1;
    }

    // ---------- block encryption ----------
    auto encryptBlock = [&](std::vector<int>& src, std::vector<int>& keyBlk) {
        std::vector<int> state(16);
        int k = 0;
        while (k < 16) {
            state[k] = src[k];
            k = k + 1;
        }

        int round = 0;
        while (round < 12) {
            int idx = 0;
            while (idx < 16) {
                int sb_in = state[idx];
                int sb_out = (round & 1) ? S2[sb_in] : S1[sb_in];
                state[idx] = sb_out;
                idx = idx + 1;
            }

            idx = 0;
            while (idx < 16) {
                int nxt = (idx + 1) & 15;
                int mix1 = state[idx] ^ state[nxt];
                int mix2 = mix1 ^ keyBlk[idx];
                state[idx] = mix2;
                idx = idx + 1;
            }

            round = round + 1;
        }

        std::vector<int> out(16);
        k = 0;
        while (k < 16) {
            out[k] = state[k];
            k = k + 1;
        }
        return out;
    };

    // ---------- OFB ----------
    std::vector<int> keystream = encryptBlock(iv, roundKey[0]);
    std::vector<int> cipher(plain_len);
    int blockPos = 0;
    while (blockPos < plain_len) {
        int byte = 0;
        while (byte < 16) {
            int pt = plain[blockPos + byte];
            int ks = keystream[byte];
            cipher[blockPos + byte] = pt ^ ks;
            byte = byte + 1;
        }
        keystream = encryptBlock(keystream, roundKey[0]);
        blockPos = blockPos + 16;
    }

    // ---------- output ----------
    std::cout << "Key      : ";
    i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (key[i] & 0xff);
        i = i + 1;
    }
    std::cout << std::dec << "\n";

    std::cout << "IV       : ";
    i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (iv[i] & 0xff);
        i = i + 1;
    }
    std::cout << std::dec << "\n";

    std::cout << "Plaintext (first 64 bytes): ";
    i = 0;
    while (i < 64) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (plain[i] & 0xff);
        i = i + 1;
    }
    std::cout << " ...\n";

    std::cout << "Ciphertext (first 64 bytes): ";
    i = 0;
    while (i < 64) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xff);
        i = i + 1;
    }
    std::cout << " ...\n";

    std::cout << std::dec << "\n";

    return 0;
}
