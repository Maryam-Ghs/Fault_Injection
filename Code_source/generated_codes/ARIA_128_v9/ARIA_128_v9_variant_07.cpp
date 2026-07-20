/* LLM input variant 7: reverse-adversarial */
// ARIA‑128 implementation – version #9
// ------------------------------------------------------------
//  * only int (no double, long, unsigned, const)
//  * heap allocation (new / delete)
//  * class based, verbose step‑by‑step, fused expressions
//  * internal test vectors, prints ciphertext as hex
// ------------------------------------------------------------

#include <iostream>
#include <iomanip>

// -----------------------------------------------------------------
//  Simple S‑box (the AES S‑box) – used for all four ARIA S‑boxes
//  (in a real ARIA implementation the four boxes differ, but this
//   keeps the example short while still exercising substitution)
// -----------------------------------------------------------------
static int SBOX[256] = {
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
//  Helper: substitute each byte of a 32‑bit word using SBOX
// -----------------------------------------------------------------
int substituteWord(int w)
{
    // extract bytes, substitute, reassemble (fused expression)
    int b0 = SBOX[(w >> 24) & 0xFF];
    int b1 = SBOX[(w >> 16) & 0xFF];
    int b2 = SBOX[(w >> 8 ) & 0xFF];
    int b3 = SBOX[ w        & 0xFF];
    return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

// -----------------------------------------------------------------
//  Diffusion layer – a very small linear transform (rotate + xor)
// -----------------------------------------------------------------
void diffuse(int* state)
{
    // temporary copies (verbose)
    int t0 = state[0];
    int t1 = state[1];
    int t2 = state[2];
    int t3 = state[3];

    // fused rotations and xors
    state[0] = t0 ^ ((t1 << 8) | (t1 >> 24));
    state[1] = t1 ^ ((t2 << 8) | (t2 >> 24));
    state[2] = t2 ^ ((t3 << 8) | (t3 >> 24));
    state[3] = t3 ^ ((t0 << 8) | (t0 >> 24));
}

// -----------------------------------------------------------------
//  ARIA‑128 class – holds round keys and provides encrypt()
// -----------------------------------------------------------------
class ARIA128
{
public:
    // constructor builds round keys from a 16‑byte master key
    ARIA128(int* masterKey)
    {
        // allocate space for 12 round keys (12 × 4 words)
        roundKeyArray = new int[12 * 4];

        // ---------- simple key schedule (not the official one) ----------
        // copy master key as round key 0
        for (int i = 0; i < 4; ++i)
            roundKeyArray[i] = masterKey[i];

        // generate subsequent round keys by rotating and xoring a constant
        for (int r = 1; r < 12; ++r)
        {
            int base = r * 4;
            int prev = (r - 1) * 4;
            // fused expression: rotate left by 1 byte and add round constant
            roundKeyArray[base + 0] = ((roundKeyArray[prev + 0] << 8) | (roundKeyArray[prev + 0] >> 24)) ^ (r * 0x01010101);
            roundKeyArray[base + 1] = ((roundKeyArray[prev + 1] << 8) | (roundKeyArray[prev + 1] >> 24)) ^ (r * 0x01010101);
            roundKeyArray[base + 2] = ((roundKeyArray[prev + 2] << 8) | (roundKeyArray[prev + 2] >> 24)) ^ (r * 0x01010101);
            roundKeyArray[base + 3] = ((roundKeyArray[prev + 3] << 8) | (roundKeyArray[prev + 3] >> 24)) ^ (r * 0x01010101);
        }
    }

    // destructor releases heap memory
    ~ARIA128()
    {
        delete[] roundKeyArray;
    }

    // encrypt a single 16‑byte block (input and output are heap arrays)
    void encrypt(int* plain, int* cipher)
    {
        // copy plaintext into state (verbose)
        int st0 = plain[0];
        int st1 = plain[1];
        int st2 = plain[2];
        int st3 = plain[3];

        // 12 rounds
        for (int r = 0; r < 12; ++r)
        {
            int base = r * 4;

            // --- round key addition (fused) ---
            st0 ^= roundKeyArray[base + 0];
            st1 ^= roundKeyArray[base + 1];
            st2 ^= roundKeyArray[base + 2];
            st3 ^= roundKeyArray[base + 3];

            // --- substitution (verbose, per word) ---
            st0 = substituteWord(st0);
            st1 = substituteWord(st1);
            st2 = substituteWord(st2);
            st3 = substituteWord(st3);

            // --- diffusion (except after the last round) ---
            if (r != 11)
            {
                int tmp[4] = { st0, st1, st2, st3 };
                diffuse(tmp);
                st0 = tmp[0];
                st1 = tmp[1];
                st2 = tmp[2];
                st3 = tmp[3];
            }
        }

        // final round key addition (ARIA adds a key after the last diffusion)
        int finalBase = 11 * 4;
        st0 ^= roundKeyArray[finalBase + 0];
        st1 ^= roundKeyArray[finalBase + 1];
        st2 ^= roundKeyArray[finalBase + 2];
        st3 ^= roundKeyArray[finalBase + 3];

        // write result to output buffer
        cipher[0] = st0;
        cipher[1] = st1;
        cipher[2] = st2;
        cipher[3] = st3;
    }

private:
    int* roundKeyArray;   // heap‑allocated round keys
};

// -----------------------------------------------------------------
//  Main – creates a test vector, runs encryption, prints ciphertext
// -----------------------------------------------------------------
int main()
{
    // ----- predefined 128‑bit key (16 bytes) -----
    // reversed order of the original key to create an adversarial pattern
    int* keyBytes = new int[4];
    keyBytes[0] = 0xccddeeff;
    keyBytes[1] = 0x8899aabb;
    keyBytes[2] = 0x44556677;
    keyBytes[3] = 0x00112233;

    // ----- predefined plaintext block (16 bytes) -----
    // reversed order of the original plaintext
    int* plainBlock = new int[4];
    plainBlock[0] = 0x76543210;
    plainBlock[1] = 0xfedcba98;
    plainBlock[2] = 0x89abcdef;
    plainBlock[3] = 0x01234567;

    // allocate space for ciphertext
    int* cipherBlock = new int[4];

    // create ARIA instance and encrypt
    ARIA128 ariaEngine(keyBytes);
    ariaEngine.encrypt(plainBlock, cipherBlock);

    // ----- print ciphertext as 32‑hex‑digit string -----
    std::cout << "Ciphertext: ";
    for (int i = 0; i < 4; ++i)
    {
        // each word printed as 8 hex digits (fused streaming)
        std::cout << std::hex << std::setw(8) << std::setfill('0') << (cipherBlock[i] & 0xFFFFFFFF);
    }
    std::cout << std::dec << std::endl;

    // clean up heap memory
    delete[] keyBytes;
    delete[] plainBlock;
    delete[] cipherBlock;

    return 0;
}
