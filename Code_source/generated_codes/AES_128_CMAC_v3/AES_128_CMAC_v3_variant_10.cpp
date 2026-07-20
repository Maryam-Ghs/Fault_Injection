#include <iostream>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

//------------------------------------------------------------
//  Helper: XOR two 128‑bit blocks (16 bytes)
//------------------------------------------------------------
void xor_block(const int* a, const int* b, int* out)
{
    // manual unrolling, four bytes per statement
    out[0] = a[0] ^ b[0]; out[1] = a[1] ^ b[1];
    out[2] = a[2] ^ b[2]; out[3] = a[3] ^ b[3];
    out[4] = a[4] ^ b[4]; out[5] = a[5] ^ b[5];
    out[6] = a[6] ^ b[6]; out[7] = a[7] ^ b[7];
    out[8] = a[8] ^ b[8]; out[9] = a[9] ^ b[9];
    out[10] = a[10] ^ b[10]; out[11] = a[11] ^ b[11];
    out[12] = a[12] ^ b[12]; out[13] = a[13] ^ b[13];
    out[14] = a[14] ^ b[14]; out[15] = a[15] ^ b[15];
}

//------------------------------------------------------------
//  Helper: left shift a 128‑bit block by one bit
//------------------------------------------------------------
void left_shift_one(const int* in, int* out)
{
    int carry = 0, next_carry;
    for (int i = 15; i >= 0; --i)
    {
        next_carry = (in[i] & 0x80) ? 1 : 0;
        out[i] = ((in[i] << 1) & 0xFF) | carry;
        carry = next_carry;
    }
}

//------------------------------------------------------------
//  Helper: multiply by x in GF(2^8) (used for subkey gen)
//------------------------------------------------------------
void generate_subkey(const int* key, int* sub)
{
    left_shift_one(key, sub);
    // if MSB of original key was 1, xor with Rb (0x87)
    if (key[0] & 0x80)
    {
        sub[15] ^= 0x87;
    }
}

//------------------------------------------------------------
//  AES constants (S‑box and multiplication tables)
//------------------------------------------------------------
static const int sbox[256] = {
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

static const int mul2[256] = {
    0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,
    32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,
    64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,
    96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,
    128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,
    160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,
    192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,
    224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,254,
    1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,
    33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,
    65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95,
    97,99,101,103,105,107,109,111,113,115,117,119,121,123,125,127,
    129,131,133,135,137,139,141,143,145,147,149,151,153,155,157,159,
    161,163,165,167,169,171,173,175,177,179,181,183,185,187,189,191,
    193,195,197,199,201,203,205,207,209,211,213,215,217,219,221,223,
    225,227,229,231,233,235,237,239,241,243,245,247,249,251,253,255
};

static const int mul3[256] = {
    0,3,6,5,12,15,10,9,24,27,30,29,20,23,18,17,
    48,51,54,53,60,63,58,57,40,43,46,45,52,55,50,49,
    96,99,102,101,108,111,106,105,120,123,126,125,116,119,114,113,
    80,83,86,85,92,95,90,89,72,75,78,77,68,71,66,65,
    192,195,198,197,204,207,202,201,216,219,222,221,212,215,210,209,
    240,243,246,245,252,255,250,249,232,235,238,237,228,231,226,225,
    128,131,134,133,140,143,138,137,152,155,158,157,148,151,146,145,
    176,179,182,181,188,191,186,185,168,171,174,173,164,167,162,161,
    3,0,5,6,15,12,9,10,27,24,29,30,23,20,17,18,
    51,48,53,54,63,60,57,58,75,72,77,78,71,68,65,66,
    99,96,101,102,111,108,105,106,123,120,125,126,119,116,113,114,
    147,144,149,150,159,156,153,154,171,168,173,174,167,164,161,162,
    195,192,197,198,207,204,201,202,219,216,221,222,215,212,209,210,
    243,240,245,246,255,252,249,250,231,228,233,234,227,224,221,222,
    131,128,133,134,143,140,137,138,155,152,157,158,151,148,145,146,
    179,176,181,182,191,188,185,186,171,168,173,174,167,164,161,162
};

//------------------------------------------------------------
//  AES core: SubBytes, ShiftRows, MixColumns, AddRoundKey
//------------------------------------------------------------
void sub_bytes(int* state)
{
    for (int i = 0; i < 16; ++i)
        state[i] = sbox[state[i]];
}

void shift_rows(int* s)
{
    // row 1 (bytes 1,5,9,13) rotate left by 1
    int t1 = s[1];
    s[1] = s[5]; s[5] = s[9]; s[9] = s[13]; s[13] = t1;

    // row 2 (bytes 2,6,10,14) rotate left by 2
    int t2 = s[2]; int t6 = s[6];
    s[2] = s[10]; s[6] = s[14]; s[10] = t2; s[14] = t6;

    // row 3 (bytes 3,7,11,15) rotate left by 3 (right by 1)
    int t3 = s[15];
    s[15] = s[11]; s[11] = s[7]; s[7] = s[3]; s[3] = t3;
}

void mix_columns(int* s)
{
    for (int c = 0; c < 4; ++c)
    {
        int i0 = 4*c, i1 = i0+1, i2 = i0+2, i3 = i0+3;
        int a0 = s[i0], a1 = s[i1], a2 = s[i2], a3 = s[i3];
        int r0 = mul2[a0] ^ mul3[a1] ^ a2 ^ a3;
        int r1 = a0 ^ mul2[a1] ^ mul3[a2] ^ a3;
        int r2 = a0 ^ a1 ^ mul2[a2] ^ mul3[a3];
        int r3 = mul3[a0] ^ a1 ^ a2 ^ mul2[a3];
        s[i0] = r0; s[i1] = r1; s[i2] = r2; s[i3] = r3;
    }
}

void add_round_key(int* state, const int* roundKey)
{
    for (int i = 0; i < 16; ++i)
        state[i] ^= roundKey[i];
}

//------------------------------------------------------------
//  Key expansion (AES‑128 => 11 round keys)
//------------------------------------------------------------
void expand_key(const int* key, int* roundKeys)
{
    // first round key is the cipher key
    for (int i = 0; i < 16; ++i)
        roundKeys[i] = key[i];

    int rcon = 1;
    for (int round = 1; round <= 10; ++round)
    {
        // generate first 4 bytes
        int t0 = roundKeys[16*round-4];
        int t1 = roundKeys[16*round-3];
        int t2 = roundKeys[16*round-2];
        int t3 = roundKeys[16*round-1];

        // RotWord
        int tmp = t0;
        t0 = t1; t1 = t2; t2 = t3; t3 = tmp;

        // SubWord
        t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];

        // Rcon
        t0 ^= rcon; rcon = mul2[rcon]; // next Rcon

        // first word of this round
        roundKeys[16*round+0] = roundKeys[16*(round-1)+0] ^ t0;
        roundKeys[16*round+1] = roundKeys[16*(round-1)+1] ^ t1;
        roundKeys[16*round+2] = roundKeys[16*(round-1)+2] ^ t2;
        roundKeys[16*round+3] = roundKeys[16*(round-1)+3] ^ t3;

        // remaining three words
        for (int i = 4; i < 16; ++i)
        {
            roundKeys[16*round+i] = roundKeys[16*(round-1)+i] ^ roundKeys[16*round+i-4];
        }
    }
}

//------------------------------------------------------------
//  AES encrypt a single 16‑byte block
//------------------------------------------------------------
void aes_encrypt(const int* in, const int* roundKeys, int* out)
{
    int state[16];
    // copy input -> state (manual unroll)
    for (int i = 0; i < 16; ++i) state[i] = in[i];

    // initial AddRoundKey
    add_round_key(state, roundKeys);

    // 9 main rounds
    for (int r = 1; r < 10; ++r)
    {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, roundKeys + 16*r);
    }

    // final round (no MixColumns)
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, roundKeys + 160);

    // copy to output
    for (int i = 0; i < 16; ++i) out[i] = state[i];
}

//------------------------------------------------------------
//  CMAC core (single‑block messages)
//------------------------------------------------------------
void aes_cmac(const int* key, const int* msg, int msg_len, int* mac)
{
    // allocate round keys on the heap
    int* roundKeys = new int[176];
    expand_key(key, roundKeys);

    // generate subkeys K1, K2
    int L[16];
    aes_encrypt(key, roundKeys, L);          // L = AES‑ENC(0^128)
    int K1[16], K2[16];
    generate_subkey(L, K1);
    generate_subkey(K1, K2);

    // decide which subkey to use
    int last_block[16];
    bool complete = (msg_len % 16) == 0 && msg_len != 0;
    int blocks = (msg_len + 15) / 16; // ceil

    // copy last block (with padding if needed)
    for (int i = 0; i < 16; ++i) last_block[i] = 0;
    int start = (blocks-1)*16;
    int remaining = msg_len - start;
    for (int i = 0; i < remaining; ++i)
        last_block[i] = msg[start + i];

    if (complete)                     // complete block → XOR K1
        xor_block(last_block, K1, last_block);
    else                              // incomplete → pad then XOR K2
    {
        last_block[remaining] = 0x80;
        xor_block(last_block, K2, last_block);
    }

    // iterate over all but last block
    int X[16]; // initially zero
    for (int i = 0; i < 16; ++i) X[i] = 0;
    int Y[16];
    for (int b = 0; b < blocks-1; ++b)
    {
        // Y = X xor M_i
        for (int i = 0; i < 16; ++i) Y[i] = X[i] ^ msg[b*16 + i];
        aes_encrypt(Y, roundKeys, X); // X = AES‑ENC(Y)
    }

    // final step: X xor last_block
    xor_block(X, last_block, Y);
    aes_encrypt(Y, roundKeys, mac); // MAC = AES‑ENC(Y)

    delete[] roundKeys;
}

//------------------------------------------------------------
//  Main – large deterministic test vector
//------------------------------------------------------------
int main()
{
    // 128‑bit key (example)
    int* key = new int[16];
    int key_vals[16] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,
                        0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
    for (int i = 0; i < 16; ++i) key[i] = key_vals[i];

    // Large message: 1024 bytes (64 blocks), deterministic pattern
    const int MSG_SIZE = 1024;
    int* msg = new int[MSG_SIZE];
    for (int i = 0; i < MSG_SIZE; ++i)
        msg[i] = i & 0xFF; // values 0..255 repeating

    int mac[16];
    aes_cmac(key, msg, MSG_SIZE, mac);

    // print MAC as hex
    std::cout << "CMAC = ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << mac[i];
    std::cout << std::dec << std::endl;

    delete[] key;
    delete[] msg;
    return 0;
}
