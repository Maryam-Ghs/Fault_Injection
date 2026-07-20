/*********************************************************************
 * AES‑192 implementation – version #6
 *   * int / float only (no unsigned, double, long, const)
 *   * class‑based, stack arrays, loop‑heavy, expanded steps
 *   * random plaintext & 192‑bit key generated internally
 * LLM input variant 1: minimal-boundary
 *********************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

class AES192
{
public:
    // -----------------------------------------------------------------
    // Constructor – builds S‑box and round constants (Rcon)
    // -----------------------------------------------------------------
    AES192()
    {
        build_sbox();
        build_rcon();
    }

    // -----------------------------------------------------------------
    // Encrypt a 16‑byte block with a 24‑byte key
    // -----------------------------------------------------------------
    void encrypt_block(const int plain[16], const int key[24], int cipher[16])
    {
        int round_key[240];                     // 4*(Nr+1)*Nb  (Nr = 12, Nb = 4)
        key_expansion(key, round_key);

        int state[4][4];
        load_state(plain, state);

        add_round_key(state, round_key, 0);

        for (int round = 1; round < 12; ++round)   // 12 rounds total
        {
            sub_bytes(state);
            shift_rows(state);
            mix_columns(state);
            add_round_key(state, round_key, round);
        }

        sub_bytes(state);
        shift_rows(state);
        add_round_key(state, round_key, 12);

        store_state(state, cipher);
    }

private:
    // -----------------------------------------------------------------
    // Tables
    // -----------------------------------------------------------------
    int sbox[256];
    int rcon[12];      // enough for 12 rounds (0‑based)

    // -----------------------------------------------------------------
    // Build S‑box (hard‑coded values, but filled at runtime)
    // -----------------------------------------------------------------
    void build_sbox()
    {
        int temp[256] = {
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
        for (int i = 0; i < 256; ++i)
            sbox[i] = temp[i];
    }

    // -----------------------------------------------------------------
    // Round constants for key schedule (Rcon[i] = x^(i-1) in GF(2^8))
    // -----------------------------------------------------------------
    void build_rcon()
    {
        int cur = 1;
        for (int i = 0; i < 12; ++i)
        {
            rcon[i] = cur;
            cur = xtime(cur);
        }
    }

    // -----------------------------------------------------------------
    // Load 16‑byte plain array into 4×4 state matrix (column‑major)
    // -----------------------------------------------------------------
    void load_state(const int plain[16], int state[4][4])
    {
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
                state[row][col] = plain[col * 4 + row];
    }

    // -----------------------------------------------------------------
    // Store state matrix back to linear array (column‑major)
    // -----------------------------------------------------------------
    void store_state(const int state[4][4], int out[16])
    {
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
                out[col * 4 + row] = state[row][col] & 0xFF;
    }

    // -----------------------------------------------------------------
    // AddRoundKey – XOR state with round key (16‑byte segment)
    // -----------------------------------------------------------------
    void add_round_key(int state[4][4], const int *rk, int round)
    {
        const int *segment = rk + round * 16;
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
                state[row][col] ^= segment[col * 4 + row];
    }

    // -----------------------------------------------------------------
    // SubBytes – byte‑wise substitution via S‑box
    // -----------------------------------------------------------------
    void sub_bytes(int state[4][4])
    {
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
                state[row][col] = sbox[state[row][col] & 0xFF];
    }

    // -----------------------------------------------------------------
    // ShiftRows – cyclic shift of each row by its index
    // -----------------------------------------------------------------
    void shift_rows(int state[4][4])
    {
        int tmp[4];
        for (int row = 1; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
                tmp[col] = state[row][(col + row) % 4];
            for (int col = 0; col < 4; ++col)
                state[row][col] = tmp[col];
        }
    }

    // -----------------------------------------------------------------
    // MixColumns – matrix multiplication in GF(2^8)
    // -----------------------------------------------------------------
    void mix_columns(int state[4][4])
    {
        for (int col = 0; col < 4; ++col)
        {
            int a0 = state[0][col];
            int a1 = state[1][col];
            int a2 = state[2][col];
            int a3 = state[3][col];

            int t0 = mul_by_2(a0) ^ mul_by_3(a1) ^ a2 ^ a3;
            int t1 = a0 ^ mul_by_2(a1) ^ mul_by_3(a2) ^ a3;
            int t2 = a0 ^ a1 ^ mul_by_2(a2) ^ mul_by_3(a3);
            int t3 = mul_by_3(a0) ^ a1 ^ a2 ^ mul_by_2(a3);

            state[0][col] = t0 & 0xFF;
            state[1][col] = t1 & 0xFF;
            state[2][col] = t2 & 0xFF;
            state[3][col] = t3 & 0xFF;
        }
    }

    // -----------------------------------------------------------------
    // Helper: multiply by 2 in GF(2^8)
    // -----------------------------------------------------------------
    int mul_by_2(int x)
    {
        int shifted = (x << 1) & 0xFF;
        if (x & 0x80)
            shifted ^= 0x1B;          // reduction polynomial
        return shifted;
    }

    // -----------------------------------------------------------------
    // Helper: multiply by 3 = x·2 ⊕ x
    // -----------------------------------------------------------------
    int mul_by_3(int x)
    {
        return mul_by_2(x) ^ (x & 0xFF);
    }

    // -----------------------------------------------------------------
    // xtime – same as mul_by_2 but returns full int (used for Rcon)
    // -----------------------------------------------------------------
    int xtime(int x)
    {
        int shifted = (x << 1);
        if (x & 0x80)
            shifted ^= 0x11B;
        return shifted & 0xFF;
    }

    // -----------------------------------------------------------------
    // KeyExpansion for 192‑bit key (24 bytes → 52 round keys of 4 words)
    // -----------------------------------------------------------------
    void key_expansion(const int key[24], int *expanded)
    {
        // Copy original key (6 words)
        for (int i = 0; i < 24; ++i)
            expanded[i] = key[i] & 0xFF;

        int bytes_generated = 24;               // 6 words * 4 bytes
        int rcon_idx = 0;

        while (bytes_generated < 240)           // allocate enough for all rounds
        {
            int temp[4];
            // take last 4‑byte word
            for (int i = 0; i < 4; ++i)
                temp[i] = expanded[bytes_generated - 4 + i];

            if (bytes_generated % 24 == 0)      // every 6th word
            {
                // RotWord
                int rot = temp[0];
                temp[0] = temp[1];
                temp[1] = temp[2];
                temp[2] = temp[3];
                temp[3] = rot;

                // SubWord
                for (int i = 0; i < 4; ++i)
                    temp[i] = sbox[temp[i] & 0xFF];

                // Rcon
                temp[0] ^= rcon[rcon_idx];
                ++rcon_idx;
            }
            else if (bytes_generated % 24 == 16) // special case for AES‑192
            {
                // SubWord only
                for (int i = 0; i < 4; ++i)
                    temp[i] = sbox[temp[i] & 0xFF];
            }

            // XOR with word 6 positions earlier
            for (int i = 0; i < 4; ++i)
            {
                int prev = expanded[bytes_generated - 24 + i];
                expanded[bytes_generated + i] = (prev ^ temp[i]) & 0xFF;
            }
            bytes_generated += 4;
        }
    }
};

// ---------------------------------------------------------------------
// Helper: print a byte array as hex
// ---------------------------------------------------------------------
void show_hex(const char *title, const int *data, int len)
{
    std::cout << title;
    for (int i = 0; i < len; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (data[i] & 0xFF) << ' ';
    std::cout << std::dec << std::endl;
}

// ---------------------------------------------------------------------
// Main – deterministic minimal‑boundary input, run AES‑192, display result
// ---------------------------------------------------------------------
int main()
{
    // Minimal deterministic input: one non‑zero byte, rest zero
    int plain[16] = {0};
    int key192[24] = {0};
    int cipher[16];

    plain[0] = 1;      // smallest non‑zero plaintext byte
    key192[0] = 1;     // smallest non‑zero key byte

    AES192 engine;
    engine.encrypt_block(plain, key192, cipher);

    show_hex("Plaintext : ", plain, 16);
    show_hex("Key       : ", key192, 24);
    show_hex("Ciphertext: ", cipher, 16);

    return 0;
}
