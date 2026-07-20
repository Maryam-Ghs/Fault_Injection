#include <iostream>
#include <iomanip>

/* LLM input variant 6: ordered-structured */

int main()
{
    /* ---------- helper lambdas (still inside main) ---------- */
    auto sub1 = [](int x) -> int { return (x ^ 0x63) & 0xFF; };                    // simple S‑box 1
    auto sub2 = [](int x) -> int { return ((x << 1) & 0xFF) | ((x >> 7) & 1); }; // simple S‑box 2

    auto mixColumns = [&](int *st)
    {
        int col = 0;
        while (col < 4)                     // four columns, loop‑heavy
        {
            int a = st[col];                 // original bytes of the column
            int b = st[4 + col];
            int c = st[8 + col];
            int d = st[12 + col];

            int e = a ^ b;                   // reordered arithmetic
            int f = c ^ d;
            int g = a + f;                   // use addition before xor (mod 2^32, but we mask later)
            int h = d + e;
            st[col]       = (g ^ 0xFF) & 0xFF;
            st[4 + col]   = (b ^ e) & 0xFF;
            st[8 + col]   = (c ^ f) & 0xFF;
            st[12 + col]  = (h ^ 0xAA) & 0xFF; // further reordering

            col = col + 1;
        }
    };

    /* ---------- predefined 256‑bit key (32 bytes) ---------- */
    int masterKey[32];
    int i = 0;
    while (i < 32)                     // ordered ascending pattern
    {
        masterKey[i] = i & 0xFF;
        i = i + 1;
    }

    /* ---------- round keys generation (13 rounds, 16‑byte each) ---------- */
    int roundKey[13][16];
    int r = 0;
    while (r < 13)
    {
        int rot = (r * 4) % 32;         // rotation amount
        int j = 0;
        while (j < 16)
        {
            int src = (rot + j) % 32;
            roundKey[r][j] = (masterKey[src] ^ (r * 0x3F)) & 0xFF; // simple diffusion in key schedule
            j = j + 1;
        }
        r = r + 1;
    }

    /* ---------- IV (initialization vector) ---------- */
    int iv[16];
    i = 0;
    while (i < 16)
    {
        iv[i] = (15 - i) & 0xFF;        // descending ordered pattern
        i = i + 1;
    }

    /* ---------- small plaintext (2 blocks = 32 bytes) ---------- */
    int plain[32];
    i = 0;
    while (i < 32)
    {
        if (i < 16)
            plain[i] = i & 0xFF;                   // first half ascending
        else
            plain[i] = (31 - i) & 0xFF;            // second half descending (symmetrical)
        i = i + 1;
    }

    /* ---------- CBC encryption ---------- */
    int cipher[32];
    int prev[16];                       // holds previous ciphertext block (or IV)
    i = 0;
    while (i < 16)
    {
        prev[i] = iv[i];
        i = i + 1;
    }

    int blockCnt = 32 / 16;
    int blkIdx = 0;
    while (blkIdx < blockCnt)
    {
        int state[16];                  // current block state on the stack
        i = 0;
        while (i < 16)                 // XOR with previous ciphertext (CBC chaining)
        {
            state[i] = (plain[blkIdx * 16 + i] ^ prev[i]) & 0xFF;
            i = i + 1;
        }

        /* ----- ARIA‑256 core (13 rounds) ----- */
        r = 0;
        while (r < 13)
        {
            i = 0;
            while (i < 16)             // add round key
            {
                state[i] = (state[i] ^ roundKey[r][i]) & 0xFF;
                i = i + 1;
            }

            i = 0;
            if ((r & 1) == 0)          // even round → sub1, odd round → sub2
            {
                while (i < 16)
                {
                    state[i] = sub1(state[i]);
                    i = i + 1;
                }
            }
            else
            {
                while (i < 16)
                {
                    state[i] = sub2(state[i]);
                    i = i + 1;
                }
            }

            if (r != 12)               // diffusion except final round
                mixColumns(state);

            r = r + 1;
        }

        /* ----- store ciphertext and prepare next block ----- */
        i = 0;
        while (i < 16)
        {
            cipher[blkIdx * 16 + i] = state[i];
            prev[i] = state[i];        // next IV = current ciphertext
            i = i + 1;
        }

        blkIdx = blkIdx + 1;
    }

    /* ---------- output ciphertext in hex ---------- */
    std::cout << "Ciphertext (CBC‑ARIA‑256, 2 blocks):\n";
    i = 0;
    while (i < 32)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xFF);
        if ((i + 1) % 16 == 0) std::cout << "\n";
        else std::cout << " ";
        i = i + 1;
    }

    return 0;
}
