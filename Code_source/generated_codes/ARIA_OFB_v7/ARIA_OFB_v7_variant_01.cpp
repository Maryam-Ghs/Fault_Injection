/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <vector>
#include <iomanip>

int main() {
    // version 7

    // ---------- helper: simple pseudo‑random generator (unused in this variant) ----------
    int seed = 987654321;
    auto next_rand = [&]() {
        seed = (seed * 1103515245 + 12345);
        return (seed >> 16) & 0x7fff;
    };

    // ---------- S‑boxes ----------
    int S1[256];
    int S2[256];
    int i = 0;
    while (i < 256) {
        S1[i] = i;                 // identity
        S2[i] = 255 - i;           // reversed
        i = i + 1;
    }

    // ---------- deterministic minimal key ----------
    std::vector<int> key = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F
    };

    // ---------- generate round keys ----------
    std::vector<std::vector<int>> roundKey(12, std::vector<int>(16));
    int r = 0;
    while (r < 12) {
        int j = 0;
        while (j < 16) {
            int idx = (j + r) & 15;               // rotate left by r
            roundKey[r][j] = key[idx];
            j = j + 1;
        }
        r = r + 1;
    }

    // ---------- deterministic minimal IV ----------
    std::vector<int> iv = {
        0xFF, 0xFE, 0xFD, 0xFC,
        0xFB, 0xFA, 0xF9, 0xF8,
        0xF7, 0xF6, 0xF5, 0xF4,
        0xF3, 0xF2, 0xF1, 0xF0
    };

    // ---------- minimal plaintext (single block, all zeros) ----------
    const int plain_len = 16;
    std::vector<int> plain(plain_len, 0x00);

    // ---------- block encryption lambda ----------
    auto encryptBlock = [&](std::vector<int>& src, std::vector<int>& keyBlk) {
        std::vector<int> state(16);
        int k = 0;
        while (k < 16) {
            state[k] = src[k];
            k = k + 1;
        }

        int round = 0;
        while (round < 12) {
            // Substitution
            int idx = 0;
            while (idx < 16) {
                int sb_in = state[idx];
                int sb_out = (round & 1) ? S2[sb_in] : S1[sb_in];
                state[idx] = sb_out;
                idx = idx + 1;
            }

            // Diffusion
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

    // ---------- OFB mode ----------
    std::vector<int> keystream = encryptBlock(iv, roundKey[0]);   // first keystream block
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

    std::cout << "Plaintext: ";
    i = 0;
    while (i < plain_len) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (plain[i] & 0xff);
        i = i + 1;
    }
    std::cout << std::dec << "\n";

    std::cout << "Ciphertext: ";
    i = 0;
    while (i < plain_len) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xff);
        i = i + 1;
    }
    std::cout << std::dec << "\n";

    return 0;
}
