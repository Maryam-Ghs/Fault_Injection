#include <iostream>
#include <iomanip>

class ARIA_XTS
{
    int rk1[32];          // round keys for data encryption
    int rk2[32];          // round keys for tweak encryption

    // simple key schedule: copy key and expand with a linear pattern
    void schedule(const int *src, int *dst)
    {
        int i = 0;
        while (i < 32)
        {
            dst[i] = src[i % 16] ^ (i * 7);
            i = i + 1;
        }
    }

    // multiply a 128‑bit tweak by α (as defined in XTS)
    void mul_alpha(int *t)
    {
        int carry = 0;
        int i = 15;
        while (i >= 0)
        {
            int next = (t[i] << 1) & 0xFF;
            t[i] = next | carry;
            carry = (t[i] & 0x80) ? 1 : 0;
            i = i - 1;
        }
        // if overflow, XOR with the primitive polynomial 0x87
        if (carry)
        {
            t[15] ^= 0x87;
        }
    }

    // one round of a toy ARIA‑like transformation
    void round_transform(int *blk, const int *rk, int round)
    {
        int i = 0;
        while (i < 16)
        {
            // fused expression: combine xor, add, and mask
            int a = (blk[i] ^ rk[round * 4 + (i % 4)]) + blk[(i + 1) % 16];
            blk[i] = a & 0xFF;
            i = i + 1;
        }
        // simple permutation: rotate left by 1 byte
        int tmp = blk[0];
        int j = 0;
        while (j < 15)
        {
            blk[j] = blk[j + 1];
            j = j + 1;
        }
        blk[15] = tmp;
    }

public:
    ARIA_XTS(const int *keyA, const int *keyB)
    {
        schedule(keyA, rk1);
        schedule(keyB, rk2);
    }

    // encrypt one 16‑byte block with given tweak
    void encrypt_block(const int *plain, int *cipher, const int *tweak)
    {
        int work[16];
        int i = 0;
        while (i < 16)
        {
            work[i] = (plain[i] ^ tweak[i]) & 0xFF;   // initial whitening
            i = i + 1;
        }

        // 4 toy rounds
        int r = 0;
        while (r < 4)
        {
            round_transform(work, rk1, r);
            r = r + 1;
        }

        i = 0;
        while (i < 16)
        {
            cipher[i] = (work[i] ^ tweak[i]) & 0xFF;   // final whitening
            i = i + 1;
        }
    }

    // encrypt a full sector (multiple of 16 bytes) in XTS mode
    void encrypt_xts(const int *data, int *out, int blocks, const int *iv)
    {
        int tweak[16];
        int i = 0;
        while (i < 16)
        {
            tweak[i] = iv[i] & 0xFF;
            i = i + 1;
        }

        // encrypt the tweak with the second key (simplified)
        int tmp[16];
        encrypt_block(tweak, tmp, rk2);   // using same encrypt routine as data
        i = 0;
        while (i < 16)
        {
            tweak[i] = tmp[i];
            i = i + 1;
        }

        int blk = 0;
        while (blk < blocks)
        {
            encrypt_block(data + blk * 16, out + blk * 16, tweak);
            mul_alpha(tweak);               // advance tweak for next block
            blk = blk + 1;
        }
    }
};

void print_hex(const int *buf, int len)
{
    int i = 0;
    while (i < len)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (buf[i] & 0xFF);
        i = i + 1;
    }
    std::cout << std::dec << std::endl;
}

/* LLM input variant 1: minimal-boundary */
int main()
{
    // ----- minimal test vector -----
    int keyA1[16]; int keyB1[16]; int iv1[16]; int pt1[16];
    int i = 0;
    while (i < 16) { keyA1[i] = 1; keyB1[i] = 1; iv1[i] = 0; i = i + 1; }
    i = 0;
    while (i < 16) { pt1[i] = 1; i = i + 1; }

    ARIA_XTS enc1(keyA1, keyB1);
    int ct1[16];
    enc1.encrypt_xts(pt1, ct1, 1, iv1);

    std::cout << "Minimal test : ";
    print_hex(ct1, 16);
    return 0;
}
