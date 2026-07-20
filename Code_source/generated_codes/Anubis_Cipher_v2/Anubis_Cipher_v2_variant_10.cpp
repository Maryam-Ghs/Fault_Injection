/* LLM input variant 10: large-safe-stress */
#include <iostream>
#include <iomanip>

int gf_mul(int a, int b)                 // Galois field multiplication (mod x^8+x^4+x^3+x+1)
{
    int p = 0;
    while (b)
    {
        p ^= (b & 1) * a;
        a <<= 1;
        if (a & 0x100) a ^= 0x11D;
        b >>= 1;
    }
    return p & 0xFF;
}

/*--- Helper: substitute bytes using the Anubis S‑box -------------------*/
void sub_bytes(int* state, const int* sbox)
{
    for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];
}

/*--- Helper: shift rows (circular left shift per row) -----------------*/
void shift_rows(int* st)
{
    int tmp;
    // row 1 (indices 1,5,9,13) shift left by 1
    tmp = st[1]; st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = tmp;
    // row 2 (indices 2,6,10,14) shift left by 2
    tmp = st[2]; st[2] = st[10]; st[10] = tmp;
    int tmp2 = st[6]; st[6] = st[14]; st[14] = tmp2;
    // row 3 (indices 3,7,11,15) shift left by 3 (right by 1)
    tmp = st[15]; st[15] = st[11]; st[11] = st[7]; st[7] = st[3]; st[3] = tmp;
}

/*--- Helper: mix columns (MDS matrix of Anubis) ----------------------*/
void mix_columns(int* st)
{
    for (int c = 0; c < 4; ++c)
    {
        int i = c * 4;
        int a0 = st[i], a1 = st[i+1], a2 = st[i+2], a3 = st[i+3];
        st[i]   = gf_mul(a0,2) ^ gf_mul(a1,1) ^ gf_mul(a2,1) ^ gf_mul(a3,3);
        st[i+1] = gf_mul(a0,3) ^ gf_mul(a1,2) ^ gf_mul(a2,1) ^ gf_mul(a3,1);
        st[i+2] = gf_mul(a0,1) ^ gf_mul(a1,3) ^ gf_mul(a2,2) ^ gf_mul(a3,1);
        st[i+3] = gf_mul(a0,1) ^ gf_mul(a1,1) ^ gf_mul(a2,3) ^ gf_mul(a3,2);
    }
}

/*--- Helper: add round key -------------------------------------------*/
void add_round_key(int* st, const int* rkey, int round)
{
    for (int i = 0; i < 16; ++i) st[i] ^= rkey[round*16 + i];
}

/*--- Helper: key schedule (128‑bit key, 12 rounds) --------------------*/
int* expand_key(const int* key)
{
    const int R = 12;                     // number of rounds for 128‑bit key
    int* rkeys = new int[(R+1)*16];       // (R+1) round keys

    // copy the original key as round‑key 0
    for (int i = 0; i < 16; ++i) rkeys[i] = key[i];

    // round constants (pre‑computed)
    static const int rc[12] = {
        0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8
    };

    // generate the remaining round keys
    for (int r = 1; r <= R; ++r)
    {
        int* prev = rkeys + (r-1)*16;
        int* cur  = rkeys + r*16;

        // first four bytes: apply the S‑box to the previous word, rotate, xor RC
        cur[0] = prev[0] ^ rc[r-1] ^ (prev[13] ^ prev[14] ^ prev[15]); // fused expression
        cur[1] = prev[1] ^ (prev[14] ^ prev[15] ^ prev[12]);
        cur[2] = prev[2] ^ (prev[15] ^ prev[12] ^ prev[13]);
        cur[3] = prev[3] ^ (prev[12] ^ prev[13] ^ prev[14]);

        // remaining words: XOR with the word four positions earlier
        for (int i = 4; i < 16; ++i) cur[i] = prev[i] ^ cur[i-4];
    }
    return rkeys;
}

/*--- Core: encrypt a single 128‑bit block -----------------------------*/
void encrypt_block(int* block, const int* sbox, const int* rkeys)
{
    const int R = 12;                                 // 12 rounds

    add_round_key(block, rkeys, 0);                   // initial key addition

    int round = 1;
    while (round <= R)
    {
        sub_bytes(block, sbox);                       // SubBytes
        shift_rows(block);                            // ShiftRows
        if (round != R) mix_columns(block);           // MixColumns (skip on last round)
        add_round_key(block, rkeys, round);           // AddRoundKey
        ++round;
    }
}

/*--- Main: deterministic large‑scale test vector ----------------------*/
int main()
{
    const int NUM_BLOCKS = 1024; // stress test: encrypt 1024 blocks

    // --- allocate and initialise S‑box on the heap --------------------
    int* sbox = new int[256];
    int preset[256] = {
        0xBA,0x54,0x2F,0x74,0x53,0xD1,0xD7,0x90,0x7B,0x81,0xB5,0xC0,0x4D,0x97,0x0A,0x3F,
        0xD3,0xA0,0x7C,0x0C,0x4E,0x5D,0x50,0x9E,0xA5,0x6E,0x3A,0xE4,0xB2,0x4B,0xA9,0x5C,
        0x9C,0x7D,0xA6,0xC1,0xB3,0x13,0xC9,0x1F,0x37,0xC8,0xDF,0x17,0x22,0x02,0x5A,0x00,
        0xC4,0x87,0x7E,0x5E,0x96,0x4C,0x7F,0x2C,0x6F,0x5B,0x9F,0x78,0x0F,0xD5,0x01,0xD8,
        0x92,0x2A,0x71,0xD0,0x07,0xF5,0x80,0x0E,0xA3,0x36,0xB0,0x9A,0xE8,0xD2,0xD6,0x5F,
        0x10,0xF8,0x25,0x7A,0x86,0xCC,0x1A,0xC5,0x3C,0x06,0xC3,0xC7,0x1D,0x2D,0x9D,0xC6,
        0xAF,0x6C,0xAE,0x0D,0x30,0x82,0x90,0xC2,0x5D,0x8B,0xF2,0x3E,0xE6,0x74,0x8C,0x9B,
        0x69,0xF0,0xB9,0x4F,0x68,0x78,0xFB,0x2B,0xA8,0x6A,0x5E,0xFA,0xE5,0x8E,0x01,0x3D,
        0x70,0x69,0x9A,0xD9,0xC1,0xC7,0x0E,0x59,0x88,0x44,0xF9,0x4D,0xB1,0x98,0x80,0xA1,
        0xB0,0x20,0x98,0x5F,0x7C,0x72,0xE3,0xE9,0x5A,0x09,0x1C,0x4B,0xED,0xA2,0xE2,0xC0,
        0x60,0x6C,0x0F,0x3F,0x78,0xE5,0x0B,0x47,0x2F,0xDB,0xF1,0xEF,0x1E,0xFB,0xF2,0x61,
        0xC8,0x5F,0x15,0xC9,0x4E,0x15,0xDC,0x0F,0x0F,0xB7,0xF6,0x9E,0x0F,0xA5,0x7B,0x8F,
        0xE9,0x1F,0x87,0x84,0x3C,0xE2,0x3F,0x5B,0xD4,0x31,0xE2,0x00,0xCB,0x8C,0x6A,0xC5,
        0xF1,0xC2,0xD7,0x9D,0xD9,0x30,0x6A,0xD0,0xC4,0xCA,0x0C,0x79,0xB8,0x33,0x0E,0x2F,
        0x6F,0x5F,0x4A,0xB8,0x4A,0x2C,0xC3,0xBC,0x2F,0x6F,0x6C,0x5C,0x89,0xE4,0x6B,0xE5,
        0x2E,0xE8,0xF0,0x51,0xD5,0x68,0xB5,0x7F,0xF3,0x34,0xA5,0x34,0x44,0xC5,0x58,0x2D
    };
    for (int i = 0; i < 256; ++i) sbox[i] = preset[i];

    // --- deterministic 128‑bit key ------------------------------------
    int* key = new int[16];
    for (int i = 0; i < 16; ++i) key[i] = i; // 00 01 02 ... 0F

    // --- expand key ----------------------------------------------------
    int* round_keys = expand_key(key);

    // --- allocate and fill plaintext blocks ------------------------------
    int* data = new int[NUM_BLOCKS * 16];
    for (int blk = 0; blk < NUM_BLOCKS; ++blk)
    {
        for (int i = 0; i < 16; ++i)
        {
            data[blk*16 + i] = ((i + blk) * 0x11) & 0xFF;
        }
    }

    // --- encrypt all blocks ---------------------------------------------
    for (int blk = 0; blk < NUM_BLOCKS; ++blk)
    {
        encrypt_block(&data[blk*16], sbox, round_keys);
    }

    // --- output ciphertexts ---------------------------------------------
    for (int blk = 0; blk < NUM_BLOCKS; ++blk)
    {
        std::cout << "Block " << blk << ": ";
        for (int i = 0; i < 16; ++i)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << data[blk*16 + i];
        std::cout << std::dec << std::endl;
    }

    // --- clean up --------------------------------------------------------
    delete[] sbox;
    delete[] key;
    delete[] data;
    delete[] round_keys;
    return 0;
}
