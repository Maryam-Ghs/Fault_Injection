#include <iostream>
#include <vector>

/* LLM input variant 3: zeros-and-ones */

int main()
{
    /* -----------------------------------------------------------------
       Simple AES‑128‑XTS implementation.
       Everything (tables, key schedule, round functions) lives inside main.
       Only int and float are used – no unsigned, long, const or double.
       ----------------------------------------------------------------- */

    /* --------------------------- S‑Box -------------------------------- */
    int sbox[256] = {
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

    /* --------------------------- Rcon --------------------------------- */
    int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

    /* --------------------------- Keys --------------------------------- */
    /* Two 128‑bit keys (16 bytes each) – zeros and ones */
    std::vector<int> key1(16, 0);            // all zeros
    std::vector<int> key2(16, 1);          // all ones

    /* -------------------------- Tweak --------------------------------- */
    /* 16‑byte tweak (sector number) – alternating zeros and ones */
    std::vector<int> tweak(16);
    for (int i = 0; i < 16; ++i) tweak[i] = i % 2;   // 0,1,0,1,...

    /* --------------------- Helper Lambdas ----------------------------- */
    auto xtime = [](int x) -> int {
        int shifted = (x << 1) & 0xFF;
        return (x & 0x80) ? (shifted ^ 0x1B) : shifted;
    };

    auto mul2 = [&](int x){ return xtime(x); };
    auto mul3 = [&](int x){ return xtime(x) ^ x; };

    /* --------------------- Key Schedule (manual unroll) -------------- */
    auto expandKey = [&](const std::vector<int>& key, std::vector< std::vector<int> >& roundKeys)
    {
        roundKeys.clear();
        roundKeys.push_back(key);                     // round 0 key

        for (int round = 1; round <= 10; ++round)
        {
            std::vector<int> prev = roundKeys[round-1];
            std::vector<int> nxt(16);

            /* ---- Key core (rotate + sbox + rcon) ---- */
            int t0 = sbox[prev[13]];
            int t1 = sbox[prev[14]];
            int t2 = sbox[prev[15]];
            int t3 = sbox[prev[12]];
            t0 ^= rcon[round];
            /* ---- First 4 bytes ---- */
            nxt[0] = prev[0] ^ t0;
            nxt[1] = prev[1] ^ t1;
            nxt[2] = prev[2] ^ t2;
            nxt[3] = prev[3] ^ t3;
            /* ---- Remaining bytes (manual unroll) ---- */
            nxt[4] = prev[4] ^ nxt[0];
            nxt[5] = prev[5] ^ nxt[1];
            nxt[6] = prev[6] ^ nxt[2];
            nxt[7] = prev[7] ^ nxt[3];
            nxt[8] = prev[8] ^ nxt[4];
            nxt[9] = prev[9] ^ nxt[5];
            nxt[10]= prev[10]^ nxt[6];
            nxt[11]= prev[11]^ nxt[7];
            nxt[12]= prev[12]^ nxt[8];
            nxt[13]= prev[13]^ nxt[9];
            nxt[14]= prev[14]^ nxt[10];
            nxt[15]= prev[15]^ nxt[11];

            roundKeys.push_back(nxt);
        }
    };

    /* --------------------- AES Round Functions ------------------------ */
    auto subBytes = [&](std::vector<int>& state)
    {
        /* Manual unrolled substitution */
        state[0] = sbox[state[0]];   state[1] = sbox[state[1]];
        state[2] = sbox[state[2]];   state[3] = sbox[state[3]];
        state[4] = sbox[state[4]];   state[5] = sbox[state[5]];
        state[6] = sbox[state[6]];   state[7] = sbox[state[7]];
        state[8] = sbox[state[8]];   state[9] = sbox[state[9]];
        state[10]= sbox[state[10]];  state[11]= sbox[state[11]];
        state[12]= sbox[state[12]];  state[13]= sbox[state[13]];
        state[14]= sbox[state[14]];  state[15]= sbox[state[15]];
    };

    auto shiftRows = [&](std::vector<int>& s)
    {
        /* Row 1 (no shift) – nothing to do */
        /* Row 2 – left rotate by 1 */
        int tmp = s[1];
        s[1] = s[5];  s[5] = s[9];  s[9] = s[13];  s[13] = tmp;
        /* Row 3 – left rotate by 2 (swap pairs) */
        std::swap(s[2], s[10]); std::swap(s[6], s[14]);
        /* Row 4 – left rotate by 3 (right rotate by 1) */
        tmp = s[15];
        s[15]= s[11]; s[11]= s[7]; s[7]= s[3]; s[3]= tmp;
    };

    auto mixColumns = [&](std::vector<int>& s)
    {
        /* Manual unrolled MixColumns – reordered arithmetic */
        for (int c = 0; c < 4; ++c)
        {
            int i = c*4;
            int a0 = s[i], a1 = s[i+1], a2 = s[i+2], a3 = s[i+3];
            int b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            s[i]   = b0; s[i+1] = b1; s[i+2] = b2; s[i+3] = b3;
        }
    };

    auto addRoundKey = [&](std::vector<int>& s, const std::vector<int>& rk)
    {
        for (int i = 0; i < 16; ++i) s[i] ^= rk[i];
    };

    auto aesEncryptBlock = [&](const std::vector<int>& in,
                               const std::vector< std::vector<int> >& roundKeys,
                               std::vector<int>& out)
    {
        out = in;                                 // copy plaintext
        addRoundKey(out, roundKeys[0]);           // initial key addition

        for (int r = 1; r < 10; ++r)
        {
            subBytes(out);
            shiftRows(out);
            mixColumns(out);
            addRoundKey(out, roundKeys[r]);
        }

        subBytes(out);
        shiftRows(out);
        addRoundKey(out, roundKeys[10]);          // final round key
    };

    /* --------------------- XTS Specific Functions -------------------- */
    auto xorBlock = [&](std::vector<int>& a, const std::vector<int>& b)
    {
        for (int i = 0; i < 16; ++i) a[i] ^= b[i];
    };

    auto incTweak = [&](std::vector<int>& t)
    {
        /* Treat as little‑endian 128‑bit integer, increment */
        for (int i = 0; i < 16; ++i)
        {
            t[i] = (t[i] + 1) & 0xFF;
            if (t[i] != 0) break;
        }
    };

    /* --------------------- Prepare round keys ------------------------ */
    std::vector< std::vector<int> > roundKeys1, roundKeys2;
    expandKey(key1, roundKeys1);
    expandKey(key2, roundKeys2);

    /* --------------------- Tweak encryption (key2) ------------------- */
    std::vector<int> tweakEnc(16);
    aesEncryptBlock(tweak, roundKeys2, tweakEnc);

    /* --------------------- Plaintext (zeros and ones) -------------- */
    std::vector<int> plaintextBlock0(16, 0);   // all zeros
    std::vector<int> plaintextBlock1(16, 1);   // all ones

    std::vector< std::vector<int> > plaintext = { plaintextBlock0, plaintextBlock1 };
    std::vector< std::vector<int> > ciphertext(2, std::vector<int>(16));

    /* --------------------- XTS Encryption Loop ---------------------- */
    std::vector<int> curTweak = tweakEnc;   // start tweak

    for (int blk = 0; blk < 2; ++blk)
    {
        std::vector<int> pt = plaintext[blk];
        xorBlock(pt, curTweak);                     // pre‑whitening
        std::vector<int> ct(16);
        aesEncryptBlock(pt, roundKeys1, ct);        // AES‑128 encrypt
        xorBlock(ct, curTweak);                     // post‑whitening
        ciphertext[blk] = ct;
        incTweak(curTweak);                         // next tweak
    }

    /* --------------------- Output ----------------------------------- */
    std::cout << "AES‑128‑XTS ciphertext (2 blocks):\n";
    for (int blk = 0; blk < 2; ++blk)
    {
        for (int i = 0; i < 16; ++i)
        {
            int v = ciphertext[blk][i];
            if (v < 16) std::cout << '0';
            std::cout << std::hex << v;
        }
        std::cout << std::dec << "\n";
    }
    return 0;
}
