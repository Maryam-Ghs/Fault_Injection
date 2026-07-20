#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

class aes256_ctr
{
public:
    // -----------------------------------------------------------------
    // Constructor: receives key (32 bytes) and iv (16 bytes)
    // -----------------------------------------------------------------
    aes256_ctr(const std::vector<int>& wKey, const std::vector<int>& wIv)
    {
        key_bytes = wKey;      // 32 elements
        iv_bytes  = wIv;       // 16 elements
        build_sbox();
        build_rcon();
        expand_key();
    }

    // -----------------------------------------------------------------
    // Encrypt a plaintext vector (multiple of 16 bytes) using CTR mode
    // -----------------------------------------------------------------
    std::vector<int> encrypt(const std::vector<int>& plain)
    {
        std::vector<int> cipher(plain.size());
        std::vector<int> counter = iv_bytes;          // current counter block
        std::vector<int> stream(16);

        int block_cnt = plain.size() / 16;
        for (int b = 0; b < block_cnt; ++b)
        {
            // generate keystream block
            encrypt_block(counter, stream);

            // xor with plaintext
            for (int i = 0; i < 16; ++i)
                cipher[b * 16 + i] = plain[b * 16 + i] ^ stream[i];

            // increment 32‑bit counter (bytes 12‑15, big endian)
            int c = (counter[12] << 24) |
                    (counter[13] << 16) |
                    (counter[14] << 8)  |
                    (counter[15]);
            ++c;
            counter[12] = (c >> 24) & 0xFF;
            counter[13] = (c >> 16) & 0xFF;
            counter[14] = (c >> 8)  & 0xFF;
            counter[15] =  c        & 0xFF;
        }
        return cipher;
    }

private:
    std::vector<int> key_bytes;   // 32
    std::vector<int> iv_bytes;    // 16
    std::vector<int> round_key;   // 240 (14+1) * 16
    std::vector<int> sbox;        // 256
    std::vector<int> rcon;        // 15

    // -----------------------------------------------------------------
    // Build the AES S‑box (hard‑coded values)
    // -----------------------------------------------------------------
    void build_sbox()
    {
        sbox = {
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
    }

    // -----------------------------------------------------------------
    // Build the Rcon array (first 15 values)
    // -----------------------------------------------------------------
    void build_rcon()
    {
        rcon = {0,1,2,4,8,16,32,64,128,27,54,108,216,171,205};
    }

    // -----------------------------------------------------------------
    // Key expansion for AES‑256 (produces 60 words = 240 bytes)
    // -----------------------------------------------------------------
    void expand_key()
    {
        round_key.assign(240, 0);
        // copy original 32‑byte key
        for (int i = 0; i < 32; ++i)
            round_key[i] = key_bytes[i];

        int w = 8;                     // we already have 8 words (32 bytes)
        while (w < 60)
        {
            int t0 = round_key[(w - 1) * 4 + 0];
            int t1 = round_key[(w - 1) * 4 + 1];
            int t2 = round_key[(w - 1) * 4 + 2];
            int t3 = round_key[(w - 1) * 4 + 3];

            if (w % 8 == 0)
            {
                // RotWord
                int tmp = t0; t0 = t1; t1 = t2; t2 = t3; t3 = tmp;
                // SubWord
                t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
                // Rcon
                t0 = t0 ^ rcon[w / 8];
            }
            else if (w % 8 == 4)
            {
                t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
            }

            int p0 = round_key[(w - 8) * 4 + 0];
            int p1 = round_key[(w - 8) * 4 + 1];
            int p2 = round_key[(w - 8) * 4 + 2];
            int p3 = round_key[(w - 8) * 4 + 3];

            round_key[w * 4 + 0] = p0 ^ t0;
            round_key[w * 4 + 1] = p1 ^ t1;
            round_key[w * 4 + 2] = p2 ^ t2;
            round_key[w * 4 + 3] = p3 ^ t3;
            ++w;
        }
    }

    // -----------------------------------------------------------------
    // Multiply by 2 in GF(2^8)
    // -----------------------------------------------------------------
    int xtime(int x)
    {
        int res = (x << 1) & 0xFF;
        if (x & 0x80) res ^= 0x1B;
        return res;
    }

    // -----------------------------------------------------------------
    // Mix one column (manual unrolling)
    // -----------------------------------------------------------------
    void mix_column(int *c)
    {
        int a0 = c[0], a1 = c[1], a2 = c[2], a3 = c[3];
        int t  = a0 ^ a1 ^ a2 ^ a3;
        int u  = a0;
        c[0] ^= t ^ xtime(a0 ^ a1);
        c[1] ^= t ^ xtime(a1 ^ a2);
        c[2] ^= t ^ xtime(a2 ^ a3);
        c[3] ^= t ^ xtime(a3 ^ u);
    }

    // -----------------------------------------------------------------
    // AddRoundKey (manual unrolled for 16 bytes)
    // -----------------------------------------------------------------
    void add_round_key(int *state, int offset)
    {
        for (int i = 0; i < 16; ++i)
            state[i] ^= round_key[offset + i];
    }

    // -----------------------------------------------------------------
    // SubBytes (manual unrolled)
    // -----------------------------------------------------------------
    void sub_bytes(int *state)
    {
        for (int i = 0; i < 16; ++i)
            state[i] = sbox[state[i]];
    }

    // -----------------------------------------------------------------
    // ShiftRows (manual unrolled)
    // -----------------------------------------------------------------
    void shift_rows(int *s)
    {
        int tmp;

        // row 1 (indexes 1,5,9,13) rotate left 1
        tmp = s[1];
        s[1] = s[5]; s[5] = s[9]; s[9] = s[13]; s[13] = tmp;

        // row 2 (indexes 2,6,10,14) rotate left 2
        tmp = s[2]; s[2] = s[10]; s[10] = tmp;
        tmp = s[6]; s[6] = s[14]; s[14] = tmp;

        // row 3 (indexes 3,7,11,15) rotate left 3 (right 1)
        tmp = s[15];
        s[15] = s[11]; s[11] = s[7]; s[7] = s[3]; s[3] = tmp;
    }

    // -----------------------------------------------------------------
    // MixColumns (manual unrolling of 4 columns)
    // -----------------------------------------------------------------
    void mix_columns(int *s)
    {
        mix_column(&s[0]);   // column 0: bytes 0,4,8,12
        mix_column(&s[4]);   // column 1: bytes 1,5,9,13  (but we stored column‑wise)
        mix_column(&s[8]);   // column 2
        mix_column(&s[12]);  // column 3
    }

    // -----------------------------------------------------------------
    // Encrypt a single 16‑byte block (standard AES‑256)
    // -----------------------------------------------------------------
    void encrypt_block(const std::vector<int>& in, std::vector<int>& out)
    {
        int s[16];
        for (int i = 0; i < 16; ++i) s[i] = in[i];

        // ---------- Round 0 ----------
        add_round_key(s, 0);

        // ---------- Rounds 1 … 13 ----------
        int r = 16;                     // offset in round_key
        for (int round = 1; round <= 13; ++round)
        {
            sub_bytes(s);
            shift_rows(s);
            mix_columns(s);
            add_round_key(s, r);
            r += 16;
        }

        // ---------- Final Round (14) ----------
        sub_bytes(s);
        shift_rows(s);
        add_round_key(s, r);            // r = 14*16

        for (int i = 0; i < 16; ++i) out[i] = s[i];
    }
};

int main()
{
    // -----------------------------------------------------------------
    // Predefined key and IV (32-byte key, 16-byte IV)
    // -----------------------------------------------------------------
    std::vector<int> key = {
        0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,
        0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,
        0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,
        0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4
    };

    std::vector<int> iv = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
    };

    // -----------------------------------------------------------------
    // Generate a large plaintext: 1024 blocks (16 KB) with a simple pattern
    // -----------------------------------------------------------------
    std::vector<int> plaintext;
    const int total_blocks = 1024; // 1024 * 16 = 16384 bytes
    plaintext.reserve(total_blocks * 16);
    for (int b = 0; b < total_blocks; ++b)
    {
        for (int i = 0; i < 16; ++i)
        {
            // Simple repeating pattern: (block_index + byte_index) mod 256
            plaintext.push_back((b + i) & 0xFF);
        }
    }

    // -----------------------------------------------------------------
    // Encrypt
    // -----------------------------------------------------------------
    aes256_ctr cipher(key, iv);
    std::vector<int> ciphertext = cipher.encrypt(plaintext);

    // -----------------------------------------------------------------
    // Output
    // -----------------------------------------------------------------
    std::cout << "Plaintext (" << plaintext.size() << " bytes):\n";
    for (size_t i = 0; i < plaintext.size(); ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (plaintext[i] & 0xFF);
        if ((i + 1) % 32 == 0) std::cout << "\n";
        else if ((i + 1) % 16 == 0) std::cout << " ";
    }
    std::cout << std::dec << "\n";

    std::cout << "Ciphertext (" << ciphertext.size() << " bytes):\n";
    for (size_t i = 0; i < ciphertext.size(); ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (ciphertext[i] & 0xFF);
        if ((i + 1) % 32 == 0) std::cout << "\n";
        else if ((i + 1) % 16 == 0) std::cout << " ";
    }
    std::cout << std::dec << "\n";

    return 0;
}
