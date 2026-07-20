#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

// -----------------------------------------------------------------
//  S‑box (plain int array, no const)
// -----------------------------------------------------------------
static int sbox[256] = {
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

// -----------------------------------------------------------------
//  Round constants (plain int array, no const)
// -----------------------------------------------------------------
static int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

// -----------------------------------------------------------------
//  Helper: multiply by 2 in GF(2^8)
// -----------------------------------------------------------------
int xtime(int x)
{
    int r = x << 1;
    if (r & 0x100) r ^= 0x11b;
    return r & 0xFF;
}

// -----------------------------------------------------------------
//  Helper: multiply by 3 in GF(2^8)  (3 = 2 ^ 1)
// -----------------------------------------------------------------
int mul3(int x)
{
    return xtime(x) ^ x;
}

// -----------------------------------------------------------------
//  SubBytes step
// -----------------------------------------------------------------
void subBytes(std::vector<int>& st)
{
    int i = 0;
    while (i < 16)
    {
        st[i] = sbox[st[i]];
        i = i + 1;
    }
}

// -----------------------------------------------------------------
//  ShiftRows step
// -----------------------------------------------------------------
void shiftRows(std::vector<int>& st)
{
    // row 1 (shift left 1)
    int tmp = st[1];
    st[1] = st[5];
    st[5] = st[9];
    st[9] = st[13];
    st[13] = tmp;

    // row 2 (shift left 2)
    tmp = st[2];
    int tmp2 = st[6];
    st[2] = st[10];
    st[6] = st[14];
    st[10] = tmp;
    st[14] = tmp2;

    // row 3 (shift left 3)
    tmp = st[3];
    st[3] = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7] = tmp;
}

// -----------------------------------------------------------------
//  MixColumns step (expanded, multi‑step)
// -----------------------------------------------------------------
void mixColumns(std::vector<int>& st)
{
    int c = 0;
    while (c < 4)
    {
        int i0 = c * 4;
        int a0 = st[i0];
        int a1 = st[i0 + 1];
        int a2 = st[i0 + 2];
        int a3 = st[i0 + 3];

        int r0 = xtime(a0) ^ mul3(a1) ^ a2 ^ a3;
        int r1 = a0 ^ xtime(a1) ^ mul3(a2) ^ a3;
        int r2 = a0 ^ a1 ^ xtime(a2) ^ mul3(a3);
        int r3 = mul3(a0) ^ a1 ^ a2 ^ xtime(a3);

        st[i0]     = r0 & 0xFF;
        st[i0 + 1] = r1 & 0xFF;
        st[i0 + 2] = r2 & 0xFF;
        st[i0 + 3] = r3 & 0xFF;

        c = c + 1;
    }
}

// -----------------------------------------------------------------
//  AddRoundKey step
// -----------------------------------------------------------------
void addRoundKey(std::vector<int>& st, const std::vector<int>& ek, int round)
{
    int i = 0;
    while (i < 16)
    {
        st[i] ^= ek[round * 16 + i];
        i = i + 1;
    }
}

// -----------------------------------------------------------------
//  Key expansion for 128‑bit key
// -----------------------------------------------------------------
void keyExpansion(const std::vector<int>& key, std::vector<int>& ek)
{
    // first 16 bytes are the original key
    int i = 0;
    while (i < 16)
    {
        ek[i] = key[i];
        i = i + 1;
    }

    int bytesGenerated = 16;
    int rconIter = 1;
    while (bytesGenerated < 176)
    {
        // take previous 4‑byte word
        int t0 = ek[bytesGenerated - 4];
        int t1 = ek[bytesGenerated - 3];
        int t2 = ek[bytesGenerated - 2];
        int t3 = ek[bytesGenerated - 1];

        // every 16‑byte boundary apply core
        if (bytesGenerated % 16 == 0)
        {
            // rotate
            int tmp = t0;
            t0 = t1;
            t1 = t2;
            t2 = t3;
            t3 = tmp;

            // subbytes
            t0 = sbox[t0];
            t1 = sbox[t1];
            t2 = sbox[t2];
            t3 = sbox[t3];

            // rcon
            t0 ^= rcon[rconIter];
            rconIter = rconIter + 1;
        }

        // XOR with word 16 bytes before
        ek[bytesGenerated]     = ek[bytesGenerated - 16] ^ t0;
        ek[bytesGenerated + 1] = ek[bytesGenerated - 15] ^ t1;
        ek[bytesGenerated + 2] = ek[bytesGenerated - 14] ^ t2;
        ek[bytesGenerated + 3] = ek[bytesGenerated - 13] ^ t3;

        bytesGenerated = bytesGenerated + 4;
    }
}

// -----------------------------------------------------------------
//  AES‑128 encryption of a single 16‑byte block
// -----------------------------------------------------------------
void aesEncryptBlock(const std::vector<int>& in, const std::vector<int>& ek, std::vector<int>& out)
{
    // copy input to state
    int i = 0;
    while (i < 16)
    {
        out[i] = in[i];
        i = i + 1;
    }

    // initial round
    addRoundKey(out, ek, 0);

    // 9 main rounds
    int round = 1;
    while (round <= 9)
    {
        subBytes(out);
        shiftRows(out);
        mixColumns(out);
        addRoundKey(out, ek, round);
        round = round + 1;
    }

    // final round (no MixColumns)
    subBytes(out);
    shiftRows(out);
    addRoundKey(out, ek, 10);
}

// -----------------------------------------------------------------
//  OFB mode encryption/decryption (same routine)
// -----------------------------------------------------------------
void aesOFB(const std::vector<int>& key,
            const std::vector<int>& iv,
            const std::vector<int>& input,
            std::vector<int>& output)
{
    // expand key once
    std::vector<int> ek(176);
    keyExpansion(key, ek);

    // current feedback block (starts with IV)
    std::vector<int> feedback = iv;

    int blockCount = (int)input.size() / 16;
    int blk = 0;
    while (blk < blockCount)
    {
        // encrypt feedback to produce keystream
        std::vector<int> keystream(16);
        aesEncryptBlock(feedback, ek, keystream);

        // XOR keystream with plaintext/ciphertext
        int j = 0;
        while (j < 16)
        {
            output[blk * 16 + j] = input[blk * 16 + j] ^ keystream[j];
            j = j + 1;
        }

        // next feedback is the keystream (as per OFB definition)
        feedback = keystream;
        blk = blk + 1;
    }
}

// -----------------------------------------------------------------
//  Helper: pretty print a byte vector as hex
// -----------------------------------------------------------------
void printHex(const std::vector<int>& data, const char* title)
{
    std::cout << title << ": ";
    int i = 0;
    while (i < (int)data.size())
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (data[i] & 0xFF);
        if ((i + 1) % 16 == 0) std::cout << ' ';
        i = i + 1;
    }
    std::cout << std::dec << std::endl;
}

// -----------------------------------------------------------------
//  Main – deterministic test vectors, everything orchestrated here
// -----------------------------------------------------------------
int main()
{
    // 128‑bit key (AES test vector)
    std::vector<int> key = {
        0x2b,0x7e,0x15,0x16,
        0x28,0xae,0xd2,0xa6,
        0xab,0xf7,0x15,0x88,
        0x09,0xcf,0x4f,0x3c
    };

    // 16‑byte IV (chosen arbitrarily but deterministic)
    std::vector<int> iv = {
        0x00,0x01,0x02,0x03,
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,
        0x0c,0x0d,0x0e,0x0f
    };

    // Plaintext – 64 blocks (1024 bytes) deterministic pattern
    const int totalBlocks = 64;
    std::vector<int> plaintext(totalBlocks * 16);
    for (int i = 0; i < (int)plaintext.size(); ++i)
    {
        // Simple pseudo‑random but deterministic pattern
        plaintext[i] = (i * 31) & 0xFF;
    }

    // Prepare buffers
    std::vector<int> ciphertext(plaintext.size());
    std::vector<int> recovered(plaintext.size());

    // Encrypt in OFB mode
    aesOFB(key, iv, plaintext, ciphertext);

    // Decrypt (same function, same IV)
    aesOFB(key, iv, ciphertext, recovered);

    // Show results
    printHex(key,        "Key");
    printHex(iv,         "IV");
    printHex(plaintext, "Plaintext");
    printHex(ciphertext,"Ciphertext (OFB)");
    printHex(recovered, "Recovered Plaintext");

    // Simple verification
    int ok = 1;
    int idx = 0;
    while (idx < (int)plaintext.size())
    {
        if ((plaintext[idx] & 0xFF) != (recovered[idx] & 0xFF))
        {
            ok = 0;
            break;
        }
        idx = idx + 1;
    }
    if (ok)
        std::cout << "Decryption successful – data matches." << std::endl;
    else
        std::cout << "Decryption failed – mismatch detected." << std::endl;

    return 0;
}
