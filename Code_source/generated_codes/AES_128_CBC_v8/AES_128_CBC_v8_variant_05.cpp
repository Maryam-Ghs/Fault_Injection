#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */
// ------------------------------------------------------------
// AES‑128 CBC – version #8 (int‑only, manual unrolled rounds)
// ------------------------------------------------------------

class aes128_cbc {
public:
    // ----------------------------------------------------------------
    // Constructor – builds round keys from a 16‑byte (int) key
    // ----------------------------------------------------------------
    aes128_cbc(const std::vector<int>& master_key) {
        key_schedule.resize(176);                    // 11 round keys × 16 bytes
        // copy the original key (first 16 bytes)
        for (int i = 0; i < 16; ++i) key_schedule[i] = master_key[i];

        int rcon = 1;
        int idx = 16;                                // next free byte in schedule

        while (idx < 176) {
            // take previous 4‑byte word
            int t0 = key_schedule[idx - 4];
            int t1 = key_schedule[idx - 3];
            int t2 = key_schedule[idx - 2];
            int t3 = key_schedule[idx - 1];

            // every 16‑byte boundary apply core transformation
            if ((idx % 16) == 0) {
                // RotWord
                int tmp = t0; t0 = t1; t1 = t2; t2 = t3; t3 = tmp;
                // SubWord
                t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
                // Rcon
                t0 = (t0 ^ rcon) & 0xFF;
                rcon = xtime(rcon);
            }

            // XOR with word 16 bytes earlier
            key_schedule[idx    ] = (key_schedule[idx - 16] ^ t0) & 0xFF;
            key_schedule[idx + 1] = (key_schedule[idx - 15] ^ t1) & 0xFF;
            key_schedule[idx + 2] = (key_schedule[idx - 14] ^ t2) & 0xFF;
            key_schedule[idx + 3] = (key_schedule[idx - 13] ^ t3) & 0xFF;

            idx += 4;
        }
    }

    // ----------------------------------------------------------------
    // Encrypt a single 16‑byte block (CBC – caller must XOR with IV)
    // ----------------------------------------------------------------
    void encrypt_block(std::vector<int>& block) const {
        // ----- Round 0 : AddRoundKey -------------------------------------------------
        add_round_key(block, 0);

        // ----- Rounds 1 … 9 (manually unrolled) ------------------------------------
        // The order of operations inside each round is deliberately shuffled
        // while keeping the mathematical result unchanged.

        // Round 1
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, 16);

        // Round 2
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, 32);

        // Round 3
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, 48);

        // Round 4
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, 64);

        // Round 5
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, 80);

        // Round 6
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, 96);

        // Round 7
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, 112);

        // Round 8
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, 128);

        // Round 9
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, 144);

        // ----- Final Round (no MixColumns) ----------------------------------------
        sub_bytes(block);
        shift_rows(block);
        add_round_key(block, 160);
    }

    // ----------------------------------------------------------------
    // CBC encrypt – deterministic single‑block test vector
    // ----------------------------------------------------------------
    std::vector<int> encrypt_cbc(const std::vector<int>& plain,
                                 const std::vector<int>& iv) const {
        std::vector<int> work = plain;               // copy plaintext

        // XOR with IV (CBC chaining)
        for (int i = 0; i < 16; ++i) work[i] = (work[i] ^ iv[i]) & 0xFF;

        encrypt_block(work);
        return work;                                 // ciphertext block
    }

private:
    std::vector<int> key_schedule;                  // 176 bytes = 11 × 16

    // ----------------------------------------------------------------
    // Helper tables (non‑const, as required)
    // ----------------------------------------------------------------
    static int sbox[256];
    static int xtime(int x) {
        return ((x << 1) ^ ((x & 0x80) ? 0x1b : 0)) & 0xFF;
    }

    // ----------------------------------------------------------------
    // Core transformations (int only)
    // ----------------------------------------------------------------
    static void sub_bytes(std::vector<int>& st) {
        for (int i = 0; i < 16; ++i) st[i] = sbox[st[i]];
    }

    static void shift_rows(std::vector<int>& st) {
        // Row 1 – shift left by 1
        int a = st[1]; st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = a;
        // Row 2 – shift left by 2 (swap pairs)
        int b = st[2]; st[2] = st[10]; st[10] = b;
        int c = st[6]; st[6] = st[14]; st[14] = c;
        // Row 3 – shift left by 3 (right by 1)
        int d = st[3]; st[3] = st[15]; st[15] = st[11]; st[11] = st[7]; st[7] = d;
    }

    static void mix_columns(std::vector<int>& st) {
        for (int c = 0; c < 4; ++c) {
            int i0 = 4 * c;
            int i1 = i0 + 1;
            int i2 = i0 + 2;
            int i3 = i0 + 3;

            int a0 = st[i0];
            int a1 = st[i1];
            int a2 = st[i2];
            int a3 = st[i3];

            // Reordered arithmetic, still equivalent to the standard MixColumns
            int t0 = (xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3) & 0xFF;
            int t1 = (a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3) & 0xFF;
            int t2 = (a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3)) & 0xFF;
            int t3 = ((xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3)) & 0xFF;

            st[i0] = t0; st[i1] = t1; st[i2] = t2; st[i3] = t3;
        }
    }

    void add_round_key(std::vector<int>& st, int offset) const {
        for (int i = 0; i < 16; ++i)
            st[i] = (st[i] ^ key_schedule[offset + i]) & 0xFF;
    }
};

// ----------------------------------------------------------------
// S‑box definition (plain int array, not const)
// ----------------------------------------------------------------
int aes128_cbc::sbox[256] = {
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

// ----------------------------------------------------------------
// Deterministic test vector (AES‑128 ECB example, used as CBC with zero IV)
// ----------------------------------------------------------------
int main() {
    // 128‑bit key (duplicate-heavy)
    std::vector<int> key = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };

    // Plaintext block (duplicate-heavy)
    std::vector<int> plain = {
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    };

    // IV for CBC (duplicate-heavy)
    std::vector<int> iv = {
        0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,
        0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA
    };

    aes128_cbc engine(key);
    std::vector<int> cipher = engine.encrypt_cbc(plain, iv);

    // Output ciphertext as hex string
    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xFF);
    std::cout << std::dec << std::endl;
    return 0;
}
