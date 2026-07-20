#include <iostream>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */

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

/*--- Main: deterministic test vector ---------------------------------*/
int main()
{
    // --- allocate and initialise S‑box on the heap (identity mapping) ----
    int* sbox = new int[256];
    for (int i = 0; i < 256; ++i) sbox[i] = i;

    // --- deterministic 128‑bit key (all zeros) and plaintext (all 0xFF) ----
    int* key = new int[16];
    int* pt  = new int[16];
    for (int i = 0; i < 16; ++i)
    {
        key[i] = 0x00;                // minimal boundary value
        pt[i]  = 0xFF;                // maximal boundary value
    }

    // --- expand key ----------------------------------------------------
    int* round_keys = expand_key(key);

    // --- encrypt --------------------------------------------------------
    encrypt_block(pt, sbox, round_keys);

    // --- output ciphertext -----------------------------------------------
    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << pt[i];
    std::cout << std::dec << std::endl;

    // --- clean up --------------------------------------------------------
    delete[] sbox;
    delete[] key;
    delete[] pt;
    delete[] round_keys;
    return 0;
}
