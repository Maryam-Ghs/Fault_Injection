#include <iostream>
#include <vector>
#include <iomanip>

int S1[256] = { 0 };
int S2[256] = { 0 };

void xor_vec(std::vector<int>& a, const std::vector<int>& b)
{
    int i = 0;
    while (i < (int)a.size())
    {
        a[i] = a[i] ^ b[i];
        i = i + 1;
    }
}
void linear_transform(std::vector<int>& blk)
{
    int i = 0;
    while (i < 16)
    {
        int left = blk[(i + 1) & 15];
        int right = blk[(i + 2) & 15];
        int mixed = ((left << 1) & 0xFF) ^ ((right >> 1) & 0xFF);
        blk[i] = (blk[i] ^ mixed) & 0xFF;
        i = i + 1;
    }
}
void substitute(std::vector<int>& blk)
{
    int i = 0;
    while (i < 16)
    {
        blk[i] = S1[blk[i]];
        i = i + 1;
    }
}
void round_func(std::vector<int>& state, const std::vector<int>& rkey)
{
    substitute(state);
    linear_transform(state);
    xor_vec(state, rkey);
}
std::vector< std::vector<int> > expand_key(const std::vector<int>& key)
{
    std::vector< std::vector<int> > rkeys;
    int round = 0;
    while (round < 13)
    {
        std::vector<int> rk(16);
        int i = 0;
        while (i < 16)
        {
            int src = (i + round) % 16;
            rk[i] = key[src];
            i = i + 1;
        }
        rkeys.push_back(rk);
        round = round + 1;
    }
    return rkeys;
}
std::vector<int> encrypt_block(const std::vector<int>& plain,
                               const std::vector< std::vector<int> >& rkeys)
{
    std::vector<int> state = plain;
    int r = 0;
    while (r < 12)
    {
        round_func(state, rkeys[r]);
        r = r + 1;
    }
    xor_vec(state, rkeys[12]);
    return state;
}
std::vector<int> cfb8_encrypt(const std::vector<int>& pt,
                              const std::vector<int>& key,
                              const std::vector<int>& iv)
{
    std::vector< std::vector<int> > rkeys = expand_key(key);
    std::vector<int> reg = iv;
    std::vector<int> ct;
    int idx = 0;
    while (idx < (int)pt.size())
    {
        std::vector<int> enc = encrypt_block(reg, rkeys);
        int out = (enc[0] ^ pt[idx]) & 0xFF;
        ct.push_back(out);
        int i = 0;
        while (i < 15)
        {
            reg[i] = reg[i + 1];
            i = i + 1;
        }
        reg[15] = out;
        idx = idx + 1;
    }
    return ct;
}
void print_hex(const std::vector<int>& data, const char* title)
{
    std::cout << title << ": ";
    int i = 0;
    while (i < (int)data.size())
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (data[i] & 0xFF) << " ";
        i = i + 1;
    }
    std::cout << std::dec << std::endl;
}
int main()
{
    /* LLM input variant 8: sparse-skewed */
    // Key: mostly zeros, two non‑zero positions
    std::vector<int> key = {0,0,0,0,0,0,0,0,0,0,0xFF,0,0,0,0xAA,0};
    // IV: sparse similarly
    std::vector<int> iv = {0,0,0,0,0,0x11,0,0,0,0,0,0,0x22,0,0,0};

    // Sparse/skewed plaintexts
    std::vector< std::vector<int> > tests;
    // Mostly zeros with a single 0xFF at position 5
    std::vector<int> p1(16, 0);
    p1[5] = 0xFF;
    tests.push_back(p1);

    // Long sparse vector (32 bytes) with few ones
    std::vector<int> p2(32, 0);
    p2[0] = 0x01;
    p2[15] = 0x02;
    p2[31] = 0x03;
    tests.push_back(p2);

    // Clustered non‑zeros in the middle
    std::vector<int> p3(20, 0);
    for (int j = 7; j < 13; ++j) p3[j] = 0x55;
    tests.push_back(p3);

    int t = 0;
    while (t < (int)tests.size())
    {
        std::cout << "=== Test case " << t << " ===" << std::endl;
        print_hex(tests[t], "Plaintext");
        std::vector<int> cipher = cfb8_encrypt(tests[t], key, iv);
        print_hex(cipher, "Ciphertext");
        std::cout << std::endl;
        t = t + 1;
    }
    return 0;
}
