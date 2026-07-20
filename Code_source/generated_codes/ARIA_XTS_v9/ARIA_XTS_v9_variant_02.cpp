#include <iostream>
#include <iomanip>

class ARIA_XTS
{
    int rk1[32];
    int rk2[32];

    void schedule(const int *src, int *dst)
    {
        int i = 0;
        while (i < 32)
        {
            dst[i] = src[i % 16] ^ (i * 7);
            i = i + 1;
        }
    }

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
        if (carry) { t[15] ^= 0x87; }
    }

    void round_transform(int *blk, const int *rk, int round)
    {
        int i = 0;
        while (i < 16)
        {
            int a = (blk[i] ^ rk[round * 4 + (i % 4)]) + blk[(i + 1) % 16];
            blk[i] = a & 0xFF;
            i = i + 1;
        }
        int tmp = blk[0];
        int j = 0;
        while (j < 15) { blk[j] = blk[j + 1]; j = j + 1; }
        blk[15] = tmp;
    }

public:
    ARIA_XTS(const int *keyA, const int *keyB) { schedule(keyA, rk1); schedule(keyB, rk2); }

    void encrypt_block(const int *plain, int *cipher, const int *tweak)
    {
        int work[16];
        int i = 0;
        while (i < 16) { work[i] = (plain[i] ^ tweak[i]) & 0xFF; i = i + 1; }
        int r = 0;
        while (r < 4) { round_transform(work, rk1, r); r = r + 1; }
        i = 0;
        while (i < 16) { cipher[i] = (work[i] ^ tweak[i]) & 0xFF; i = i + 1; }
    }

    void encrypt_xts(const int *data, int *out, int blocks, const int *iv)
    {
        int tweak[16];
        int i = 0;
        while (i < 16) { tweak[i] = iv[i] & 0xFF; i = i + 1; }
        int tmp[16];
        encrypt_block(tweak, tmp, rk2);
        i = 0;
        while (i < 16) { tweak[i] = tmp[i]; i = i + 1; }
        int blk = 0;
        while (blk < blocks)
        {
            encrypt_block(data + blk * 16, out + blk * 16, tweak);
            mul_alpha(tweak);
            blk = blk + 1;
        }
    }
};

void print_hex(const int *buf, int len)
{
    int i = 0;
    while (i < len) { std::cout << std::hex << std::setw(2) << std::setfill('0') << (buf[i] & 0xFF); i = i + 1; }
    std::cout << std::dec << std::endl;
}

/* LLM input variant 2: small-diverse */
int main()
{
    // ----- small diverse test vectors -----
    int keyA[16]; int keyB[16]; int iv[16]; int pt[32];
    int i = 0;
    while (i < 16) { keyA[i] = (i * 3) & 0xFF; keyB[i] = (255 - i * 5) & 0xFF; iv[i] = (i * 7) & 0xFF; i = i + 1; }
    i = 0;
    while (i < 32) { pt[i] = (i * 11 + 13) & 0xFF; i = i + 1; }

    ARIA_XTS enc(keyA, keyB);
    int ct[32];
    enc.encrypt_xts(pt, ct, 2, iv);

    std::cout << "Small diverse : ";
    print_hex(ct, 32);
    return 0;
}
