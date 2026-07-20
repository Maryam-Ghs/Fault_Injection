/* LLM input variant 2: small-diverse */
// AES-128/CBC implementation – Version #9
// ------------------------------------------------------------
// Constraints fulfilled:
//   – Only int (no double, long, unsigned, const)
//   – Uses std::vector for all dynamic data
//   – Class‑based design with loop‑heavy iterative steps
//   – Multi‑step expanded arithmetic for each AES operation
//   – Generates edge‑case heavy inputs internally
// ------------------------------------------------------------

#include <iostream>
#include <vector>
#include <iomanip>

class AES_CBC_128 {
public:
    // Constructor builds the round keys from a 16‑byte key
    AES_CBC_128(const std::vector<int>& raw_key) {
        build_round_keys(raw_key);
    }

    // Encrypt a series of 16‑byte blocks using CBC mode
    std::vector<std::vector<int>> encrypt_cbc(
        const std::vector<std::vector<int>>& plain_blocks,
        const std::vector<int>& iv_block)
    {
        std::vector<std::vector<int>> cipher_blocks;
        std::vector<int> prev = iv_block; // chaining value

        int blk_idx = 0;
        while (blk_idx < (int)plain_blocks.size()) {
            // XOR with previous ciphertext (or IV for first block)
            std::vector<int> mixed = xor_vectors(plain_blocks[blk_idx], prev);
            // Encrypt the mixed block
            std::vector<int> encrypted = encrypt_one_block(mixed);
            // Store ciphertext and update chaining value
            cipher_blocks.push_back(encrypted);
            prev = encrypted;
            blk_idx = blk_idx + 1;
        }
        return cipher_blocks;
    }

private:
    // 11 round keys, each 16 bytes
    std::vector<std::vector<int>> round_keys;

    // ----------------------------------------------------
    // Helper: XOR two vectors of equal length (byte‑wise)
    // ----------------------------------------------------
    std::vector<int> xor_vectors(const std::vector<int>& a,
                                 const std::vector<int>& b) {
        std::vector<int> out;
        int i = 0;
        while (i < (int)a.size()) {
            int x = a[i] ^ b[i];
            out.push_back(x & 0xFF);
            i = i + 1;
        }
        return out;
    }

    // ----------------------------------------------------
    // Build all round keys from the original 16‑byte key
    // ----------------------------------------------------
    void build_round_keys(const std::vector<int>& master_key) {
        // Rcon values (only first 10 needed)
        int rcon_vals[10] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

        // First round key is the master key itself
        round_keys.push_back(master_key);

        int round = 0;
        while (round < 10) {
            // ------- Core word generation -------
            std::vector<int> temp;
            // take last 4 bytes of previous round key
            int j = 12;
            while (j < 16) {
                temp.push_back(round_keys[round][j]);
                j = j + 1;
            }
            // RotWord: rotate left by one byte
            int first = temp[0];
            int k = 0;
            while (k < 3) {
                temp[k] = temp[k+1];
                k = k + 1;
            }
            temp[3] = first;
            // SubWord: apply S‑box to each byte
            int m = 0;
            while (m < 4) {
                temp[m] = sbox_lookup(temp[m]);
                m = m + 1;
            }
            // Rcon XOR to first byte
            temp[0] = temp[0] ^ rcon_vals[round];

            // ------- Expand the 16‑byte round key -------
            std::vector<int> new_key;
            int i = 0;
            while (i < 16) {
                int prev_byte = round_keys[round][i];
                int add_byte = temp[i % 4];
                int new_byte = prev_byte ^ add_byte;
                new_key.push_back(new_byte & 0xFF);
                // update temp for next 4‑byte chunk
                if ((i % 4) == 3) {
                    int t = 0;
                    while (t < 4) {
                        temp[t] = new_key[i - 3 + t];
                        t = t + 1;
                    }
                }
                i = i + 1;
            }
            round_keys.push_back(new_key);
            round = round + 1;
        }
    }

    // ----------------------------------------------------
    // Encrypt a single 16‑byte block (iteration style)
    // ----------------------------------------------------
    std::vector<int> encrypt_one_block(const std::vector<int>& plain) {
        // copy plain into state
        std::vector<int> state = plain;

        // Initial AddRoundKey
        state = add_round_key(state, round_keys[0]);

        int round = 1;
        while (round <= 9) {
            // SubBytes
            state = sub_bytes(state);
            // ShiftRows
            state = shift_rows(state);
            // MixColumns
            state = mix_columns(state);
            // AddRoundKey
            state = add_round_key(state, round_keys[round]);
            round = round + 1;
        }

        // Final round (no MixColumns)
        state = sub_bytes(state);
        state = shift_rows(state);
        state = add_round_key(state, round_keys[10]);

        return state;
    }

    // ----------------------------------------------------
    // AddRoundKey: XOR state with round key
    // ----------------------------------------------------
    std::vector<int> add_round_key(const std::vector<int>& st,
                                   const std::vector<int>& rk) {
        std::vector<int> out;
        int i = 0;
        while (i < 16) {
            out.push_back((st[i] ^ rk[i]) & 0xFF);
            i = i + 1;
        }
        return out;
    }

    // ----------------------------------------------------
    // SubBytes: byte‑wise S‑box substitution
    // ----------------------------------------------------
    std::vector<int> sub_bytes(const std::vector<int>& st) {
        std::vector<int> out;
        int i = 0;
        while (i < 16) {
            out.push_back(sbox_lookup(st[i]));
            i = i + 1;
        }
        return out;
    }

    // ----------------------------------------------------
    // ShiftRows: row‑wise cyclic shift
    // ----------------------------------------------------
    std::vector<int> shift_rows(const std::vector<int>& st) {
        // state is column‑major (AES spec)
        std::vector<int> out(16);
        int r = 0;
        while (r < 4) {
            int c = 0;
            while (c < 4) {
                int src_idx = ((c + r) % 4) * 4 + r;
                int dst_idx = c * 4 + r;
                out[dst_idx] = st[src_idx];
                c = c + 1;
            }
            r = r + 1;
        }
        return out;
    }

    // ----------------------------------------------------
    // MixColumns: column‑wise GF(2^8) multiplication
    // ----------------------------------------------------
    std::vector<int> mix_columns(const std::vector<int>& st) {
        std::vector<int> out(16);
        int col = 0;
        while (col < 4) {
            int idx0 = col * 4;
            int idx1 = idx0 + 1;
            int idx2 = idx0 + 2;
            int idx3 = idx0 + 3;

            int a0 = st[idx0];
            int a1 = st[idx1];
            int a2 = st[idx2];
            int a3 = st[idx3];

            // Perform the multiplication steps explicitly
            int t0 = xtime(a0) ^ mul3(a1) ^ a2 ^ a3;
            int t1 = a0 ^ xtime(a1) ^ mul3(a2) ^ a3;
            int t2 = a0 ^ a1 ^ xtime(a2) ^ mul3(a3);
            int t3 = mul3(a0) ^ a1 ^ a2 ^ xtime(a3);

            out[idx0] = t0 & 0xFF;
            out[idx1] = t1 & 0xFF;
            out[idx2] = t2 & 0xFF;
            out[idx3] = t3 & 0xFF;

            col = col + 1;
        }
        return out;
    }

    // ----------------------------------------------------
    // Helper: multiply by 2 in GF(2^8)
    // ----------------------------------------------------
    int xtime(int v) {
        int shifted = (v << 1) & 0xFF;
        if ((v & 0x80) != 0) {
            shifted = shifted ^ 0x1B;
        }
        return shifted & 0xFF;
    }

    // Helper: multiply by 3 = xtime(x) ^ x
    int mul3(int v) {
        return (xtime(v) ^ v) & 0xFF;
    }

    // ----------------------------------------------------
    // S‑box lookup (hard‑coded table)
    // ----------------------------------------------------
    int sbox_lookup(int byte_val) {
        static std::vector<int> sbox = {
            0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
            0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
            0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
            0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
            0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
            0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
            0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
            0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
            0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
            0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
            0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
            0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
            0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
            0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
            0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
            0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
        };
        return sbox[byte_val & 0xFF];
    }
};

// ------------------------------------------------------------
// Helper: pretty‑print a block as hex
// ------------------------------------------------------------
void print_hex_block(const std::vector<int>& blk) {
    int i = 0;
    while (i < (int)blk.size()) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (blk[i] & 0xFF);
        if (i % 4 == 3) std::cout << " ";
        i = i + 1;
    }
    std::cout << std::dec << std::endl;
}

// ------------------------------------------------------------
// Main: generate edge‑case heavy inputs, encrypt, and display
// ------------------------------------------------------------
int main() {
    // Fixed 128‑bit key (different from original)
    std::vector<int> key = {
        0x00,0x01,0x02,0x03,
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,
        0x0C,0x0D,0x0E,0x0F
    };

    // IV with a simple incrementing pattern
    std::vector<int> iv = {
        0x10,0x11,0x12,0x13,
        0x14,0x15,0x16,0x17,
        0x18,0x19,0x1A,0x1B,
        0x1C,0x1D,0x1E,0x1F
    };

    // Small diverse plaintext blocks
    std::vector<std::vector<int>> plain_blocks;

    // 1) All 0xAA
    std::vector<int> block_aa(16, 0xAA);
    plain_blocks.push_back(block_aa);

    // 2) All 0x55
    std::vector<int> block_55(16, 0x55);
    plain_blocks.push_back(block_55);

    // 3) Descending bytes 0x0F..0x00
    std::vector<int> block_desc;
    int d = 15;
    while (d >= 0) {
        block_desc.push_back(d);
        d = d - 1;
    }
    plain_blocks.push_back(block_desc);

    // 4) Alternating pattern 0x0C,0x0D,0x0C,0x0D...
    std::vector<int> block_alt;
    int alt_cnt = 0;
    while (alt_cnt < 16) {
        block_alt.push_back((alt_cnt % 2 == 0) ? 0x0C : 0x0D);
        alt_cnt = alt_cnt + 1;
    }
    plain_blocks.push_back(block_alt);

    // 5) Fixed pseudo‑random block
    std::vector<int> block_rand = {
        0xDE,0xAD,0xBE,0xEF,
        0xCA,0xFE,0xBA,0xBE,
        0xFA,0xCE,0xB0,0x0C,
        0x12,0x34,0x56,0x78
    };
    plain_blocks.push_back(block_rand);

    // Create cipher object
    AES_CBC_128 cipher(key);

    // Encrypt all blocks
    std::vector<std::vector<int>> cipher_blocks = cipher.encrypt_cbc(plain_blocks, iv);

    // Output results
    std::cout << "Ciphertext blocks (hex, CBC mode):" << std::endl;
    int blk = 0;
    while (blk < (int)cipher_blocks.size()) {
        std::cout << "Block " << blk << ": ";
        print_hex_block(cipher_blocks[blk]);
        blk = blk + 1;
    }

    return 0;
}
