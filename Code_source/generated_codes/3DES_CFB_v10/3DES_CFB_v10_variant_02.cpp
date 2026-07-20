#include <iostream>
#include <vector>
#include <cstdio>

/* LLM input variant 2: small-diverse */
int main()
{
    // ------------------------------------------------------------
    //  Tables (no const, only int)
    // ------------------------------------------------------------
    int ip[64] = {
        58,50,42,34,26,18,10,2,
        60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6,
        64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9,1,
        59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5,
        63,55,47,39,31,23,15,7
    };
    int fp[64] = {
        40,8,48,16,56,24,64,32,
        39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30,
        37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28,
        35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26,
        33,1,41,9,49,17,57,25
    };
    int ebox[48] = {
        32,1,2,3,4,5,4,5,6,7,8,9,
        8,9,10,11,12,13,12,13,14,15,16,17,
        16,17,18,19,20,21,20,21,22,23,24,25,
        24,25,26,27,28,29,28,29,30,31,32,1
    };
    int pbox[32] = {
        16,7,20,21,29,12,28,17,
        1,15,23,26,5,18,31,10,
        2,8,24,14,32,27,3,9,
        19,13,30,6,22,11,4,25
    };
    int sbox[8][4][16] = {
        {{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
         {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
         {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
         {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}},
        {{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
         {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
         {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
         {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}},
        {{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
         {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
         {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
         {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}},
        {{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
         {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
         {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
         {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}},
        {{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
         {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
         {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
         {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}},
        {{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
         {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
         {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
         {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}},
        {{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
         {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
         {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
         {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}}
    };
    int pc1[56] = {
        57,49,41,33,25,17,9,
        1,58,50,42,34,26,18,
        10,2,59,51,43,35,27,
        19,11,3,60,52,44,36,
        63,55,47,39,31,23,15,
        7,62,54,46,38,30,22,
        14,6,61,53,45,37,29,
        21,13,5,28,20,12,4
    };
    int pc2[48] = {
        14,17,11,24,1,5,
        3,28,15,6,21,10,
        23,19,12,4,26,8,
        16,7,27,20,13,2,
        41,52,31,37,47,55,
        30,40,51,45,33,48,
        44,49,39,56,34,53,
        46,42,50,36,29,32
    };
    int rot[16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

    // ------------------------------------------------------------
    //  Helper lambdas (all inside main)
    // ------------------------------------------------------------
    auto perm = [&](const std::vector<int>& src, const int* tbl, int sz)->std::vector<int>
    {
        std::vector<int> dst(sz);
        int i = 0;
        while(i < sz)
        {
            dst[i] = src[tbl[i]-1];
            i = i + 1;
        }
        return dst;
    };

    auto leftShift = [&](std::vector<int>& half, int cnt)
    {
        int i = 0;
        while(i < cnt)
        {
            int first = half[0];
            int j = 0;
            while(j < (int)half.size()-1)
            {
                half[j] = half[j+1];
                j = j + 1;
            }
            half[half.size()-1] = first;
            i = i + 1;
        }
    };

    auto genSub = [&](const std::vector<int>& key)->std::vector< std::vector<int> >
    {
        std::vector< std::vector<int> > out(16);
        std::vector<int> pc1key = perm(key, pc1, 56);
        std::vector<int> c(pc1key.begin(), pc1key.begin()+28);
        std::vector<int> d(pc1key.begin()+28, pc1key.end());

        int r = 0;
        while(r < 16)
        {
            leftShift(c, rot[r]);
            leftShift(d, rot[r]);
            std::vector<int> cd;
            cd.insert(cd.end(), c.begin(), c.end());
            cd.insert(cd.end(), d.begin(), d.end());
            out[r] = perm(cd, pc2, 48);
            r = r + 1;
        }
        return out;
    };

    auto feistel = [&](const std::vector<int>& rhalf, const std::vector<int>& subkey)->std::vector<int>
    {
        std::vector<int> expanded = perm(rhalf, ebox, 48);
        int i = 0;
        while(i < 48)
        {
            expanded[i] = expanded[i] ^ subkey[i];
            i = i + 1;
        }

        std::vector<int> out(32);
        int s = 0;
        while(s < 8)
        {
            int row = (expanded[s*6] << 1) | expanded[s*6+5];
            int col = (expanded[s*6+1] << 3) |
                      (expanded[s*6+2] << 2) |
                      (expanded[s*6+3] << 1) |
                       expanded[s*6+4];
            int val = sbox[s][row][col];
            out[s*4]   = (val >> 3) & 1;
            out[s*4+1] = (val >> 2) & 1;
            out[s*4+2] = (val >> 1) & 1;
            out[s*4+3] =  val       & 1;
            s = s + 1;
        }
        out = perm(out, pbox, 32);
        return out;
    };

    auto desBlock = [&](const std::vector<int>& block,
                        const std::vector< std::vector<int> >& subkeys,
                        bool decrypt)->std::vector<int>
    {
        std::vector<int> ipt = perm(block, ip, 64);
        std::vector<int> left(ipt.begin(), ipt.begin()+32);
        std::vector<int> right(ipt.begin()+32, ipt.end());

        int r = 0;
        while(r < 16)
        {
            std::vector<int> temp = right;
            std::vector<int> f = feistel(right, decrypt ? subkeys[15-r] : subkeys[r]);
            int i = 0;
            while(i < 32)
            {
                right[i] = left[i] ^ f[i];
                i = i + 1;
            }
            left = temp;
            r = r + 1;
        }

        // swap one more time
        std::vector<int> preout;
        preout.insert(preout.end(), right.begin(), right.end());
        preout.insert(preout.end(), left.begin(), left.end());

        std::vector<int> final = perm(preout, fp, 64);
        return final;
    };

    auto tripleDes = [&](const std::vector<int>& blk,
                         const std::vector< std::vector<int> >& k1,
                         const std::vector< std::vector<int> >& k2,
                         const std::vector< std::vector<int> >& k3,
                         bool decrypt)->std::vector<int>
    {
        // EDE: encrypt‑k1, decrypt‑k2, encrypt‑k3
        std::vector<int> step1 = desBlock(blk, k1, false);
        std::vector<int> step2 = desBlock(step1, k2, true);
        std::vector<int> step3 = desBlock(step2, k3, false);
        return step3;
    };

    // ------------------------------------------------------------
    //  Deterministic test data (small-diverse)
    // ------------------------------------------------------------
    int blocks = 4; // four 64‑bit blocks = 32 bytes
    std::vector<int> plain = {
        0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
        0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,
        0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,
        0x90,0xA0,0xB0,0xC0,0xD0,0xE0,0xF0,0x01
    };

    std::vector<int> kA = {0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF};
    std::vector<int> kB = {0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10};
    std::vector<int> kC = {0x0F,0x1E,0x2D,0x3C,0x4B,0x5A,0x69,0x78};

    std::vector<int> iv = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};

    // ------------------------------------------------------------
    //  Prepare subkeys (bits)
    // ------------------------------------------------------------
    auto byte2bits = [&](const std::vector<int>& bytes)->std::vector<int>
    {
        std::vector<int> bits;
        int b = 0;
        while(b < (int)bytes.size())
        {
            int mask = 128;
            while(mask > 0)
            {
                bits.push_back( (bytes[b] & mask) ? 1 : 0 );
                mask = mask >> 1;
            }
            b = b + 1;
        }
        return bits;
    };
    auto bits2byte = [&](const std::vector<int>& bits)->std::vector<int>
    {
        std::vector<int> bytes;
        int pos = 0;
        while(pos < (int)bits.size())
        {
            int val = 0;
            int j = 0;
            while(j < 8)
            {
                val = (val << 1) | bits[pos+j];
                j = j + 1;
            }
            bytes.push_back(val);
            pos = pos + 8;
        }
        return bytes;
    };

    std::vector<int> kb1 = byte2bits(kA);
    std::vector<int> kb2 = byte2bits(kB);
    std::vector<int> kb3 = byte2bits(kC);
    std::vector< std::vector<int> > sub1 = genSub(kb1);
    std::vector< std::vector<int> > sub2 = genSub(kb2);
    std::vector< std::vector<int> > sub3 = genSub(kb3);

    // ------------------------------------------------------------
    //  CFB encryption
    // ------------------------------------------------------------
    std::vector<int> cipher;
    std::vector<int> curIV = iv;
    int blkIdx = 0;
    while(blkIdx < blocks)
    {
        // take plaintext block
        std::vector<int> pblk(plain.begin()+blkIdx*8, plain.begin()+(blkIdx+1)*8);
        // encrypt IV with 3DES
        std::vector<int> ivbits = byte2bits(curIV);
        std::vector<int> streamBits = tripleDes(ivbits, sub1, sub2, sub3, false);
        std::vector<int> stream = bits2byte(streamBits);
        // XOR
        std::vector<int> cblk(8);
        int j = 0;
        while(j < 8)
        {
            cblk[j] = pblk[j] ^ stream[j];
            j = j + 1;
        }
        // output
        cipher.insert(cipher.end(), cblk.begin(), cblk.end());
        // next IV = ciphertext block
        curIV = cblk;
        blkIdx = blkIdx + 1;
    }

    // ------------------------------------------------------------
    //  Print results
    // ------------------------------------------------------------
    std::cout << "Plaintext : ";
    int i = 0;
    while(i < (int)plain.size())
    {
        std::printf("%02X ", plain[i]);
        i = i + 1;
    }
    std::cout << "\nCiphertext: ";
    i = 0;
    while(i < (int)cipher.size())
    {
        std::printf("%02X ", cipher[i]);
        i = i + 1;
    }
    std::cout << std::endl;

    return 0;
}
