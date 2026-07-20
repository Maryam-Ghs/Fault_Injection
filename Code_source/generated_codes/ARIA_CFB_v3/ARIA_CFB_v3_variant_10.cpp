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
    /* LLM input variant 10: large-safe-stress */
    // Key: pattern 0xAA,0x55 repeated
    std::vector<int> key(16);
    int i = 0;
    while (i < 16)
    {
        key[i] = (i % 2 == 0) ? 0xAA : 0x55;
        i = i + 1;
    }

    // IV: incremental pattern 0x00..0x0F
    std::vector<int> iv(16);
    i = 0;
    while (i < 16)
    {
        iv[i] = i;
        i = i + 1;
    }

    // Large plaintext: 1024 bytes deterministic pattern
    std::vector<int> pt(1024);
    i = 0;
    while (i < 1024)
    {
        pt[i] = (i * 7 + 3) & 0xFF;
        i = i + 1;
    }

    std::cout << "=== Test case 0 (large) ===" << std::endl;
    print_hex(pt, "Plaintext (first 64 bytes)");
    // For brevity, only print first 64 bytes of ciphertext
    std::vector<int> cipher = cfb8_encrypt(pt, key, iv);
    std::vector<int> first64(cipher.begin(), cipher.begin() + 64);
    print_hex(first64, "Ciphertext (first 64 bytes)");
    std::cout << std::endl;
    return 0;
}
