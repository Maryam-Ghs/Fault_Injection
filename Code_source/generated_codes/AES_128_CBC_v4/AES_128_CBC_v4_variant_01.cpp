/********************************************************************
 *  AES‑128 CBC encryption – version #4
 *
 *  Constraints fulfilled:
 *   • Only int (no double, long, unsigned, const)
 *   • Heap allocation with new / delete
 *   • Verbose, step‑by‑step implementation
 *   • Expanded multi‑step arithmetic
 *   • Class based design
 *   • Random large input arrays generated internally
 *   • No use of std::cin
 ********************************************************************/

#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */

/* ----------------------------------------------------------------- *
 *  Helper: multiplication by x in GF(2^8) (the xtime operation)      *
 * ----------------------------------------------------------------- */
int xtime(int value)
{
    int shifted = (value << 1) & 0xFF;               // keep only low 8 bits
    if ((value & 0x80) != 0)                         // if high bit was set
    {
        shifted ^= 0x1B;                             // reduce modulo the AES polynomial
    }
    return shifted;
}

/* ----------------------------------------------------------------- *
 *  AES‑128 class – CBC mode                                        *
 * ----------------------------------------------------------------- */
class AES128_CBC
{
public:
    AES128_CBC(int *key_bytes);          // constructor – expands key
    ~AES128_CBC();                       // destructor – frees expanded key

    void encrypt(int *plain, int plain_len, int *iv, int *cipher);
    // encrypts `plain_len` bytes (multiple of 16) using CBC, writes to `cipher`

private:
    int *exp_key;                        // 176‑byte expanded key on the heap

    void key_expansion(int *key);
    void add_round_key(int state[4][4], int round);
    void sub_bytes(int state[4][4]);
    void shift_rows(int state[4][4]);
    void mix_columns(int state[4][4]);
    void encrypt_one_block(int *in_block, int *out_block, int block_id);
    void xor_with_vector(int *block, int *vector);

    static int sbox[256];
    static int rcon[11];
};

/* ----------------------------------------------------------------- *
 *  Static tables (S‑box and Rcon) – not const per the constraints   *
 * ----------------------------------------------------------------- */
int AES128_CBC::sbox[256] = {
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

int AES128_CBC::rcon[11] = {
    0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36
};

/* ----------------------------------------------------------------- *
 *  Constructor – copies the 16‑byte key and expands it               *
 * ----------------------------------------------------------------- */
AES128_CBC::AES128_CBC(int *key_bytes)
{
    // Allocate space for 44 words (4 bytes each) = 176 ints
    exp_key = new int[176];

    // Copy original key (first 16 bytes) into the first 16 positions
    int idx = 0;
    while (idx < 16)
    {
        exp_key[idx] = key_bytes[idx];
        idx = idx + 1;
    }

    // Perform key expansion to fill the remaining 160 bytes
    key_expansion(key_bytes);
}

/* ----------------------------------------------------------------- *
 *  Destructor – frees the expanded key memory                        *
 * ----------------------------------------------------------------- */
AES128_CBC::~AES128_CBC()
{
    delete [] exp_key;
}

/* ----------------------------------------------------------------- *
 *  Key expansion – AES‑128 specific (44 words)                       *
 * ----------------------------------------------------------------- */
void AES128_CBC::key_expansion(int *key)
{
    int bytes_generated = 16;                // we already have 16 bytes
    int rcon_iteration = 1;                  // Rcon starts at index 1
    int temp_word[4];                        // temporary storage for a word

    while (bytes_generated < 176)
    {
        // ---- fetch previous word -----------------------------------
        int i = 0;
        while (i < 4)
        {
            temp_word[i] = exp_key[bytes_generated - 4 + i];
            i = i + 1;
        }

        // ---- every 16 bytes apply the core transformation ------------
        if ((bytes_generated % 16) == 0)
        {
            // RotWord – rotate left
            int first = temp_word[0];
            int j = 0;
            while (j < 3)
            {
                temp_word[j] = temp_word[j + 1];
                j = j + 1;
            }
            temp_word[3] = first;

            // SubWord – apply S‑box to each byte
            int k = 0;
            while (k < 4)
            {
                int original = temp_word[k];
                int substituted = sbox[original];
                temp_word[k] = substituted;
                k = k + 1;
            }

            // XOR with Rcon
            temp_word[0] = temp_word[0] ^ rcon[rcon_iteration];
            rcon_iteration = rcon_iteration + 1;
        }

        // ---- XOR with word 16 bytes before ---------------------------
        int j = 0;
        while (j < 4)
        {
            int before = exp_key[bytes_generated - 16 + j];
            int new_byte = before ^ temp_word[j];
            exp_key[bytes_generated] = new_byte;
            bytes_generated = bytes_generated + 1;
            j = j + 1;
        }
    }
}

/* ----------------------------------------------------------------- *
 *  AddRoundKey – XOR state with round key                           *
 * ----------------------------------------------------------------- */
void AES128_CBC::add_round_key(int state[4][4], int round)
{
    int col = 0;
    while (col < 4)
    {
        int row = 0;
        while (row < 4)
        {
            int key_index = (round * 16) + (col * 4) + row;
            state[row][col] = state[row][col] ^ exp_key[key_index];
            row = row + 1;
        }
        col = col + 1;
    }
}

/* ----------------------------------------------------------------- *
 *  SubBytes – apply S‑box to every state byte                        *
 * ----------------------------------------------------------------- */
void AES128_CBC::sub_bytes(int state[4][4])
{
    int r = 0;
    while (r < 4)
    {
        int c = 0;
        while (c < 4)
        {
            int byte_val = state[r][c];
            int sb = sbox[byte_val];
            state[r][c] = sb;
            c = c + 1;
        }
        r = r + 1;
    }
}

/* ----------------------------------------------------------------- *
 *  ShiftRows – cyclically shift rows                                 *
 * ----------------------------------------------------------------- */
void AES128_CBC::shift_rows(int state[4][4])
{
    // Row 1 – shift left by 1
    int temp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = temp;

    // Row 2 – shift left by 2
    int temp1 = state[2][0];
    int temp2 = state[2][1];
    state[2][0] = state[2][2];
    state[2][1] = state[2][3];
    state[2][2] = temp1;
    state[2][3] = temp2;

    // Row 3 – shift left by 3 (or right by 1)
    temp = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = temp;
}

/* ----------------------------------------------------------------- *
 *  MixColumns – matrix multiplication in GF(2^8)                    *
 * ----------------------------------------------------------------- */
void AES128_CBC::mix_columns(int state[4][4])
{
    int c = 0;
    while (c < 4)
    {
        int a0 = state[0][c];
        int a1 = state[1][c];
        int a2 = state[2][c];
        int a3 = state[3][c];

        int r0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        int r1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        int r2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        int r3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);

        state[0][c] = r0 & 0xFF;
        state[1][c] = r1 & 0xFF;
        state[2][c] = r2 & 0xFF;
        state[3][c] = r3 & 0xFF;

        c = c + 1;
    }
}

/* ----------------------------------------------------------------- *
 *  XOR a 16‑byte block with a 16‑byte vector (IV or previous cipher) *
 * ----------------------------------------------------------------- */
void AES128_CBC::xor_with_vector(int *block, int *vector)
{
    int i = 0;
    while (i < 16)
    {
        block[i] = block[i] ^ vector[i];
        i = i + 1;
    }
}

/* ----------------------------------------------------------------- *
 *  Encrypt a single 16‑byte block                                    *
 * ----------------------------------------------------------------- */
void AES128_CBC::encrypt_one_block(int *in_block, int *out_block, int block_id)
{
    // ---- load block into state matrix (column‑major) ---------------
    int state[4][4];
    int idx = 0;
    while (idx < 16)
    {
        int column = idx / 4;
        int row    = idx % 4;
        state[row][column] = in_block[idx];
        idx = idx + 1;
    }

    // ---- initial AddRoundKey ---------------------------------------
    add_round_key(state, 0);

    // ---- 9 main rounds ---------------------------------------------
    int round = 1;
    while (round <= 9)
    {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, round);
        round = round + 1;
    }

    // ---- final round (no MixColumns) -------------------------------
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, 10);

    // ---- write state back to output block (column‑major) ----------
    idx = 0;
    while (idx < 16)
    {
        int column = idx / 4;
        int row    = idx % 4;
        out_block[idx] = state[row][column] & 0xFF;
        idx = idx + 1;
    }
}

/* ----------------------------------------------------------------- *
 *  CBC encryption of an entire buffer                                 *
 * ----------------------------------------------------------------- */
void AES128_CBC::encrypt(int *plain, int plain_len, int *iv, int *cipher)
{
    // Number of 16‑byte blocks
    int total_blocks = plain_len / 16;
    int block_index = 0;

    // Allocate a temporary buffer for the current plaintext block
    int *current_block = new int[16];

    // ---- iterate over all blocks ------------------------------------
    while (block_index < total_blocks)
    {
        // ---- copy plaintext of this block into temporary buffer -------
        int i = 0;
        while (i < 16)
        {
            current_block[i] = plain[block_index * 16 + i];
            i = i + 1;
        }

        // ---- XOR with IV (or previous ciphertext) --------------------
        if (block_index == 0)
        {
            xor_with_vector(current_block, iv);
        }
        else
        {
            xor_with_vector(current_block, &cipher[(block_index - 1) * 16]);
        }

        // ---- encrypt the block ----------------------------------------
        encrypt_one_block(current_block, &cipher[block_index * 16], block_index);

        block_index = block_index + 1;
    }

    delete [] current_block;
}

/* ----------------------------------------------------------------- *
 *  Main – generate deterministic key, IV, plaintext and run encryption *
 * ----------------------------------------------------------------- */
int main()
{
    // ---- deterministic 16‑byte key -------------------------------
    int *key = new int[16];
    int i = 0;
    while (i < 16)
    {
        key[i] = i;               // simple incremental values 0..15
        i = i + 1;
    }

    // ---- deterministic 16‑byte IV --------------------------------
    int *iv = new int[16];
    i = 0;
    while (i < 16)
    {
        iv[i] = 15 - i;           // descending values 15..0
        i = i + 1;
    }

    // ---- minimal plaintext: exactly one block (16 bytes) ----------
    const int plain_size = 16;          // must be multiple of 16
    int *plaintext = new int[plain_size];
    i = 0;
    while (i < plain_size)
    {
        plaintext[i] = (i * 3) % 256; // deterministic pattern
        i = i + 1;
    }

    // ---- allocate space for ciphertext --------------------------------
    int *ciphertext = new int[plain_size];

    // ---- create AES object and encrypt ---------------------------------
    AES128_CBC aes_instance(key);
    aes_instance.encrypt(plaintext, plain_size, iv, ciphertext);

    // ---- print results -------------------------------------------------
    std::cout << "Plaintext (hex):" << std::endl;
    i = 0;
    while (i < plain_size)
    {
        printf("%02X ", plaintext[i]);
        if ((i + 1) % 16 == 0) std::cout << std::endl;
        i = i + 1;
    }

    std::cout << "\nCiphertext (hex):" << std::endl;
    i = 0;
    while (i < plain_size)
    {
        printf("%02X ", ciphertext[i]);
        if ((i + 1) % 16 == 0) std::cout << std::endl;
        i = i + 1;
    }

    // ---- clean up ------------------------------------------------------
    delete [] key;
    delete [] iv;
    delete [] plaintext;
    delete [] ciphertext;

    return 0;
}
