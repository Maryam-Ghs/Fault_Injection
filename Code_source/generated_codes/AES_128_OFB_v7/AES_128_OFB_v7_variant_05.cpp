/* LLM input variant 5: duplicate-heavy */
// AES-128-OFB implementation – version #7
// ------------------------------------------------------------
// Restrictions applied:
//   * Only int (no double, long, unsigned, const)
//   * All dynamic memory via new/delete
//   * No cin, input generated internally
//   * Verbose step‑by‑step with many temporaries
//   * Helper functions, heap arrays, random large input
// ------------------------------------------------------------

#include <iostream>
#include <cstdlib>
#include <ctime>

// ------------------------------------------------------------
// Helper: simple pseudo‑random byte generator (0‑255)
// ------------------------------------------------------------
int randomByte()
{
    return std::rand() % 256;
}

// ------------------------------------------------------------
// AES tables (int, not const)
// ------------------------------------------------------------
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

int rcon[10] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36 };

// ------------------------------------------------------------
// GF(2^8) multiplication helpers (int only)
// ------------------------------------------------------------
int mul2(int x)
{
    int shifted = x << 1;
    int reduced = (x & 0x80) ? (shifted ^ 0x1B) : shifted;
    return reduced & 0xFF;
}
int mul3(int x)
{
    return (mul2(x) ^ x) & 0xFF;
}

// ------------------------------------------------------------
// AddRoundKey – XOR state with round key (16 bytes)
// ------------------------------------------------------------
void addRoundKey(int* arrState, int* arrRoundKey)
{
    int idx = 0;
    while (idx < 16)
    {
        int a = arrState[idx];
        int b = arrRoundKey[idx];
        int c = a ^ b;
        arrState[idx] = c & 0xFF;
        idx = idx + 1;
    }
}

// ------------------------------------------------------------
// SubBytes – byte substitution using S‑box
// ------------------------------------------------------------
void subBytes(int* arrState)
{
    int i = 0;
    while (i < 16)
    {
        int src = arrState[i];
        int sub = sbox[src];
        arrState[i] = sub & 0xFF;
        i = i + 1;
    }
}

// ------------------------------------------------------------
// ShiftRows – cyclic left shift per row
// ------------------------------------------------------------
void shiftRows(int* arrState)
{
    // Row 0 untouched
    // Row 1 shift by 1
    int t1 = arrState[1];
    int t5 = arrState[5];
    int t9 = arrState[9];
    int t13 = arrState[13];
    arrState[1]  = t5;
    arrState[5]  = t9;
    arrState[9]  = t13;
    arrState[13] = t1;

    // Row 2 shift by 2
    int t2 = arrState[2];
    int t6 = arrState[6];
    int t10 = arrState[10];
    int t14 = arrState[14];
    arrState[2]  = t10;
    arrState[6]  = t14;
    arrState[10] = t2;
    arrState[14] = t6;

    // Row 3 shift by 3 (right shift by 1)
    int t3 = arrState[3];
    int t7 = arrState[7];
    int t11 = arrState[11];
    int t15 = arrState[15];
    arrState[3]  = t15;
    arrState[7]  = t3;
    arrState[11] = t7;
    arrState[15] = t11;
}

// ------------------------------------------------------------
// MixColumns – column wise diffusion
// ------------------------------------------------------------
void mixColumns(int* arrState)
{
    int col = 0;
    while (col < 4)
    {
        int base = col * 4;               // column start index
        int s0 = arrState[base + 0];
        int s1 = arrState[base + 1];
        int s2 = arrState[base + 2];
        int s3 = arrState[base + 3];

        int m0 = mul2(s0) ^ mul3(s1) ^ s2 ^ s3;
        int m1 = s0 ^ mul2(s1) ^ mul3(s2) ^ s3;
        int m2 = s0 ^ s1 ^ mul2(s2) ^ mul3(s3);
        int m3 = mul3(s0) ^ s1 ^ s2 ^ mul2(s3);

        arrState[base + 0] = m0 & 0xFF;
        arrState[base + 1] = m1 & 0xFF;
        arrState[base + 2] = m2 & 0xFF;
        arrState[base + 3] = m3 & 0xFF;

        col = col + 1;
    }
}

// ------------------------------------------------------------
// Key Expansion – generate 44 words (176 bytes) from 16‑byte key
// ------------------------------------------------------------
void keyExpansion(int* arrKey, int* arrExpanded)
{
    // Copy initial key (first 4 words)
    int i = 0;
    while (i < 16)
    {
        arrExpanded[i] = arrKey[i] & 0xFF;
        i = i + 1;
    }

    // Generate remaining words
    int wordIdx = 4; // each word = 4 bytes
    while (wordIdx < 44)
    {
        // Temp holds previous word
        int t0 = arrExpanded[(wordIdx - 1) * 4 + 0];
        int t1 = arrExpanded[(wordIdx - 1) * 4 + 1];
        int t2 = arrExpanded[(wordIdx - 1) * 4 + 2];
        int t3 = arrExpanded[(wordIdx - 1) * 4 + 3];

        // If wordIdx mod 4 == 0, apply core schedule
        if ((wordIdx % 4) == 0)
        {
            // RotWord
            int r0 = t1;
            int r1 = t2;
            int r2 = t3;
            int r3 = t0;

            // SubWord
            int s0 = sbox[r0];
            int s1 = sbox[r1];
            int s2 = sbox[r2];
            int s3 = sbox[r3];

            // Rcon xor on first byte
            int rc = rcon[(wordIdx / 4) - 1];
            int rcByte = rc & 0xFF;
            int n0 = s0 ^ rcByte;

            // Updated temp
            t0 = n0 & 0xFF;
            t1 = s1 & 0xFF;
            t2 = s2 & 0xFF;
            t3 = s3 & 0xFF;
        }

        // XOR with word four positions earlier
        int w0 = arrExpanded[(wordIdx - 4) * 4 + 0];
        int w1 = arrExpanded[(wordIdx - 4) * 4 + 1];
        int w2 = arrExpanded[(wordIdx - 4) * 4 + 2];
        int w3 = arrExpanded[(wordIdx - 4) * 4 + 3];

        arrExpanded[wordIdx * 4 + 0] = (w0 ^ t0) & 0xFF;
        arrExpanded[wordIdx * 4 + 1] = (w1 ^ t1) & 0xFF;
        arrExpanded[wordIdx * 4 + 2] = (w2 ^ t2) & 0xFF;
        arrExpanded[wordIdx * 4 + 3] = (w3 ^ t3) & 0xFF;

        wordIdx = wordIdx + 1;
    }
}

// ------------------------------------------------------------
// AES encryption of a single 16‑byte block
// ------------------------------------------------------------
void aesEncryptBlock(int* arrIn, int* arrExpKey, int* arrOut)
{
    // Copy input to state (heap)
    int* arrState = new int[16];
    int i = 0;
    while (i < 16)
    {
        arrState[i] = arrIn[i] & 0xFF;
        i = i + 1;
    }

    // Initial AddRoundKey (round 0)
    addRoundKey(arrState, arrExpKey);

    // 9 main rounds
    int round = 1;
    while (round <= 9)
    {
        subBytes(arrState);
        shiftRows(arrState);
        mixColumns(arrState);
        int* roundKeyPtr = arrExpKey + round * 16;
        addRoundKey(arrState, roundKeyPtr);
        round = round + 1;
    }

    // Final round (no MixColumns)
    subBytes(arrState);
    shiftRows(arrState);
    int* finalKeyPtr = arrExpKey + 10 * 16;
    addRoundKey(arrState, finalKeyPtr);

    // Write out ciphertext
    int j = 0;
    while (j < 16)
    {
        arrOut[j] = arrState[j] & 0xFF;
        j = j + 1;
    }

    delete[] arrState;
}

// ------------------------------------------------------------
// OFB mode encryption (plaintext -> ciphertext)
// ------------------------------------------------------------
void ofbEncrypt(int* arrPlain, int plainLen, int* arrKey, int* arrIV, int* arrCipher)
{
    // Expand key once
    int* arrExpKey = new int[176];
    keyExpansion(arrKey, arrExpKey);

    // Feedback buffer (starts with IV)
    int* arrFeedback = new int[16];
    int idx = 0;
    while (idx < 16)
    {
        arrFeedback[idx] = arrIV[idx] & 0xFF;
        idx = idx + 1;
    }

    // Process each 16‑byte block
    int blockPos = 0;
    while (blockPos < plainLen)
    {
        // Generate keystream block by encrypting feedback
        int* arrKeystream = new int[16];
        aesEncryptBlock(arrFeedback, arrExpKey, arrKeystream);

        // XOR plaintext with keystream to produce ciphertext
        int inner = 0;
        while (inner < 16 && (blockPos + inner) < plainLen)
        {
            int pt = arrPlain[blockPos + inner];
            int ks = arrKeystream[inner];
            arrCipher[blockPos + inner] = (pt ^ ks) & 0xFF;
            inner = inner + 1;
        }

        // Prepare next feedback (the keystream becomes new feedback)
        int copyIdx = 0;
        while (copyIdx < 16)
        {
            arrFeedback[copyIdx] = arrKeystream[copyIdx] & 0xFF;
            copyIdx = copyIdx + 1;
        }

        delete[] arrKeystream;
        blockPos = blockPos + 16;
    }

    delete[] arrFeedback;
    delete[] arrExpKey;
}

// ------------------------------------------------------------
// Main – generate deterministic duplicate‑heavy key, IV, plaintext; encrypt and print
// ------------------------------------------------------------
int main()
{
    std::srand(static_cast<unsigned>(std::time(0)));

    // ---------- Key ----------
    int* arrKey = new int[16];
    int i = 0;
    while (i < 16)
    {
        arrKey[i] = 0xAA; // duplicate value 170
        i = i + 1;
    }

    // ---------- IV ----------
    int* arrIV = new int[16];
    i = 0;
    while (i < 16)
    {
        arrIV[i] = 0xBB; // duplicate value 187
        i = i + 1;
    }

    // ---------- Plaintext (duplicate‑heavy array) ----------
    const int plainSize = 1024;               // 64 blocks
    int* arrPlain = new int[plainSize];
    i = 0;
    while (i < plainSize)
    {
        arrPlain[i] = 0x00; // all zeros
        i = i + 1;
    }

    // ---------- Ciphertext buffer ----------
    int* arrCipher = new int[plainSize];

    // ---------- Perform OFB encryption ----------
    ofbEncrypt(arrPlain, plainSize, arrKey, arrIV, arrCipher);

    // ---------- Output first 64 ciphertext bytes ----------
    std::cout << "Ciphertext (first 64 bytes, hex):\n";
    int outIdx = 0;
    while (outIdx < 64)
    {
        int byteVal = arrCipher[outIdx];
        // print as two‑digit hex
        const char* hexDigits = "0123456789ABCDEF";
        char hi = hexDigits[(byteVal >> 4) & 0x0F];
        char lo = hexDigits[byteVal & 0x0F];
        std::cout << hi << lo << ' ';
        outIdx = outIdx + 1;
    }
    std::cout << std::endl;

    // Clean up
    delete[] arrKey;
    delete[] arrIV;
    delete[] arrPlain;
    delete[] arrCipher;

    return 0;
}
