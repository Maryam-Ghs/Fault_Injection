#include <iostream>
#include <iomanip>
#include <cstdint>

/* LLM input variant 9: medium-deterministic-random */

// ------------------------------------------------------------
// Helper functions (heap allocated tables)
// ------------------------------------------------------------
int* create_sbox()
{
    int* sb = new int[256];
    int i = 0;
    while (i < 256)
    {
        sb[i] = i;
        ++i;
    }
    // Hard‑coded AES S‑box (values fit in signed int)
    int raw[256] = {
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
    i = 0;
    while (i < 256)
    {
        sb[i] = raw[i];
        ++i;
    }
    return sb;
}

int* create_rcon()
{
    int* rc = new int[15];
    int i = 1;
    int idx = 1;
    while (idx < 15)
    {
        rc[idx] = i;
        i = (i << 1) ^ ((i & 0x80) ? 0x11b : 0);
        i = i & 0xFF;
        ++idx;
    }
    rc[0] = 0;
    return rc;
}

// ------------------------------------------------------------
// Simple deterministic pseudo‑random generator (LCG)
// ------------------------------------------------------------
void lcg_fill(int* arr, int count, uint32_t seed)
{
    uint32_t state = seed;
    int i = 0;
    while (i < count)
    {
        state = (state * 1664525u + 1013904223u) & 0xFFFFFFFFu;
        arr[i] = static_cast<int>(state & 0xFF);
        ++i;
    }
}

// ------------------------------------------------------------
// Core AES operations (operate on 16‑byte state)
// ------------------------------------------------------------
void sub_bytes(int* st, int* sb)
{
    int i = 0;
    while (i < 16)
    {
        st[i] = sb[st[i]];
        ++i;
    }
}

void shift_rows(int* st)
{
    int tmp;
    // Row 1 (shift left 1)
    tmp = st[1];
    st[1] = st[5];
    st[5] = st[9];
    st[9] = st[13];
    st[13] = tmp;
    // Row 2 (shift left 2)
    tmp = st[2];
    st[2] = st[10];
    st[10] = tmp;
    tmp = st[6];
    st[6] = st[14];
    st[14] = tmp;
    // Row 3 (shift left 3)
    tmp = st[3];
    st[3] = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7] = tmp;
}

int xtime(int x)
{
    int r = x << 1;
    r = r ^ ((r & 0x100) ? 0x11b : 0);
    return r & 0xFF;
}

void mix_columns(int* st)
{
    int i = 0;
    while (i < 4)
    {
        int a0 = st[4*i];
        int a1 = st[4*i+1];
        int a2 = st[4*i+2];
        int a3 = st[4*i+3];

        int t = a0 ^ a1 ^ a2 ^ a3;
        int u = a0;

        st[4*i]   = a0 ^ t ^ xtime(a0 ^ a1);
        st[4*i+1] = a1 ^ t ^ xtime(a1 ^ a2);
        st[4*i+2] = a2 ^ t ^ xtime(a2 ^ a3);
        st[4*i+3] = a3 ^ t ^ xtime(a3 ^ u);

        ++i;
    }
}

void add_round_key(int* st, int* rk)
{
    int i = 0;
    while (i < 16)
    {
        st[i] = st[i] ^ rk[i];
        ++i;
    }
}

// ------------------------------------------------------------
// Key schedule for AES‑256 (14 rounds)
// ------------------------------------------------------------
void key_expansion(int* key, int* rks, int* sb, int* rc)
{
    int i = 0;
    while (i < 8)                // first 8 words (32 bytes)
    {
        rks[4*i]   = key[4*i];
        rks[4*i+1] = key[4*i+1];
        rks[4*i+2] = key[4*i+2];
        rks[4*i+3] = key[4*i+3];
        ++i;
    }

    int bytes = 32;
    int r = 1;
    while (bytes < 240)          // (14+1)*16 = 240 bytes
    {
        int temp0 = rks[bytes-4];
        int temp1 = rks[bytes-3];
        int temp2 = rks[bytes-2];
        int temp3 = rks[bytes-1];

        if ((bytes / 4) % 8 == 0)
        {
            // RotWord
            int t = temp0;
            temp0 = temp1;
            temp1 = temp2;
            temp2 = temp3;
            temp3 = t;
            // SubWord
            temp0 = sb[temp0];
            temp1 = sb[temp1];
            temp2 = sb[temp2];
            temp3 = sb[temp3];
            // Rcon
            temp0 = temp0 ^ rc[r];
            ++r;
        }
        else if ((bytes / 4) % 8 == 4)
        {
            // SubWord only
            temp0 = sb[temp0];
            temp1 = sb[temp1];
            temp2 = sb[temp2];
            temp3 = sb[temp3];
        }

        int prev0 = rks[bytes - 32];
        int prev1 = rks[bytes - 31];
        int prev2 = rks[bytes - 30];
        int prev3 = rks[bytes - 29];

        rks[bytes]   = prev0 ^ temp0;
        rks[bytes+1] = prev1 ^ temp1;
        rks[bytes+2] = prev2 ^ temp2;
        rks[bytes+3] = prev3 ^ temp3;

        bytes += 4;
    }
}

// ------------------------------------------------------------
// Single‑block AES encryption
// ------------------------------------------------------------
void aes_encrypt_block(int* in, int* out, int* rks, int* sb)
{
    int state[16];
    int i = 0;
    while (i < 16)
    {
        state[i] = in[i];
        ++i;
    }

    add_round_key(state, rks);               // round 0
    int round = 1;
    while (round < 14)
    {
        sub_bytes(state, sb);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, rks + 16*round);
        ++round;
    }
    // final round
    sub_bytes(state, sb);
    shift_rows(state);
    add_round_key(state, rks + 16*14);

    i = 0;
    while (i < 16)
    {
        out[i] = state[i];
        ++i;
    }
}

// ------------------------------------------------------------
// CMAC subkey generation (K1, K2)
// ------------------------------------------------------------
void left_shift_one(int* in, int* out)
{
    int carry = 0;
    int i = 15;
    while (i >= 0)
    {
        int cur = in[i];
        out[i] = ((cur << 1) & 0xFF) | carry;
        carry = (cur & 0x80) ? 1 : 0;
        --i;
    }
}

void generate_subkeys(int* key, int* K1, int* K2, int* rks, int* sb, int* rc)
{
    int zero[16];
    int i = 0;
    while (i < 16)
    {
        zero[i] = 0;
        ++i;
    }
    int L[16];
    aes_encrypt_block(zero, L, rks, sb);

    left_shift_one(L, K1);
    if (L[0] & 0x80)
    {
        K1[15] = K1[15] ^ 0x87;
    }

    left_shift_one(K1, K2);
    if (K1[0] & 0x80)
    {
        K2[15] = K2[15] ^ 0x87;
    }
}

// ------------------------------------------------------------
// CMAC computation
// ------------------------------------------------------------
void xor_block(int* a, int* b, int* out)
{
    int i = 0;
    while (i < 16)
    {
        out[i] = a[i] ^ b[i];
        ++i;
    }
}

void cmac_compute(int* key, int* msg, int msg_len, int* tag, int* rks, int* sb, int* rc)
{
    int K1[16];
    int K2[16];
    generate_subkeys(key, K1, K2, rks, sb, rc);

    int n = (msg_len + 15) / 16;          // number of blocks (ceil)
    int last_complete = (msg_len % 16) == 0 ? 1 : 0;

    int M_last[16];
    int i = 0;
    if (last_complete)
    {
        // last block is complete -> XOR with K1
        int offset = (n-1)*16;
        while (i < 16)
        {
            M_last[i] = msg[offset + i] ^ K1[i];
            ++i;
        }
    }
    else
    {
        // padding then XOR with K2
        int offset = (n-1)*16;
        while (i < 16)
        {
            if (i < (msg_len % 16))
                M_last[i] = msg[offset + i];
            else if (i == (msg_len % 16))
                M_last[i] = 0x80;
            else
                M_last[i] = 0;
            ++i;
        }
        i = 0;
        while (i < 16)
        {
            M_last[i] = M_last[i] ^ K2[i];
            ++i;
        }
    }

    int X[16];
    i = 0;
    while (i < 16)
    {
        X[i] = 0;
        ++i;
    }

    int block[16];
    int blk_idx = 0;
    while (blk_idx < n-1)
    {
        int offset = blk_idx*16;
        i = 0;
        while (i < 16)
        {
            block[i] = msg[offset + i];
            ++i;
        }
        xor_block(X, block, X);
        aes_encrypt_block(X, X, rks, sb);
        ++blk_idx;
    }

    xor_block(X, M_last, X);
    aes_encrypt_block(X, tag, rks, sb);
}

// ------------------------------------------------------------
// Main – deterministic test vector (variant 9)
// ------------------------------------------------------------
int main()
{
    // Allocate tables on heap
    int* sbox = create_sbox();
    int* rcon = create_rcon();

    // 256‑bit key – pseudo‑random deterministic values
    int key[32];
    lcg_fill(key, 32, 0x5A5A5A5Au);

    // Message – 78 bytes of pseudo‑random data (not a multiple of 16)
    int msg_len = 78;
    int* message = new int[msg_len];
    lcg_fill(message, msg_len, 0xC3C3C3C3u);

    // Prepare round keys
    int* roundKeys = new int[240];   // (14+1)*16
    key_expansion(key, roundKeys, sbox, rcon);

    // Compute CMAC
    int mac[16];
    cmac_compute(key, message, msg_len, mac, roundKeys, sbox, rcon);

    // Output
    std::cout << "CMAC-256: ";
    int i = 0;
    while (i < 16)
    {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (mac[i] & 0xFF);
        ++i;
    }
    std::cout << std::dec << std::endl;

    // Clean up
    delete[] sbox;
    delete[] rcon;
    delete[] message;
    delete[] roundKeys;
    return 0;
}
