#include <iostream>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

int main()
{
    /*-------------------  AES‑256 tables  -------------------*/
    int Sbox[256] = {
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
    int Rcon[15] = {1,2,4,8,16,32,64,128,27,54,108,216,171,205,123};

    /*-------------------  Test vectors  -------------------*/
    int key[32] = {
        0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,
        0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4
    };
    int nonce[16] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
    };
    const int PLAINTEXT_SIZE = 256; // 16 blocks of 16 bytes each
    int plain[PLAINTEXT_SIZE] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
        0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
        0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
        0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
        0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
        0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
        0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
        0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
        0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
        0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
        0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
        0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
        0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
        0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
        0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
    };

    /*-------------------  Helper lambdas  -------------------*/
    auto xtime = [&](int b)->int{
        int hi = (b >> 7) & 1;
        int res = ((b << 1) & 0xff) ^ (0x1b * hi);
        return res;
    };

    auto mixColumn = [&](int *c){
        int a0=c[0], a1=c[1], a2=c[2], a3=c[3];
        int t = a0 ^ a1 ^ a2 ^ a3;
        int u = a0;
        c[0] ^= t ^ xtime(a0 ^ a1);
        c[1] ^= t ^ xtime(a1 ^ a2);
        c[2] ^= t ^ xtime(a2 ^ a3);
        c[3] ^= t ^ xtime(a3 ^ u);
    };

    auto subBytes = [&](int *st){
        int i=0;
        while(i<16){
            st[i] = Sbox[st[i]];
            ++i;
        }
    };

    auto shiftRows = [&](int *st){
        int tmp;
        // row 1
        tmp = st[1]; st[1]=st[5]; st[5]=st[9]; st[9]=st[13]; st[13]=tmp;
        // row 2
        tmp = st[2]; st[2]=st[10]; st[10]=tmp;
        tmp = st[6]; st[6]=st[14]; st[14]=tmp;
        // row 3
        tmp = st[3]; st[3]=st[15]; st[15]=st[11]; st[11]=st[7]; st[7]=tmp;
    };

    auto addRoundKey = [&](int *st, int *rk){
        int i=0;
        while(i<16){
            st[i] ^= rk[i];
            ++i;
        }
    };

    auto keyExpand = [&](int *k, int *rk){
        int i=0;
        while(i<8){
            rk[4*i+0]=k[4*i+0];
            rk[4*i+1]=k[4*i+1];
            rk[4*i+2]=k[4*i+2];
            rk[4*i+3]=k[4*i+3];
            ++i;
        }
        i=8;
        while(i<60){
            int temp[4];
            temp[0]=rk[4*(i-1)+0];
            temp[1]=rk[4*(i-1)+1];
            temp[2]=rk[4*(i-1)+2];
            temp[3]=rk[4*(i-1)+3];
            if(i%8==0){
                // RotWord
                int rot = temp[0];
                temp[0]=temp[1]; temp[1]=temp[2]; temp[2]=temp[3]; temp[3]=rot;
                // SubWord
                temp[0]=Sbox[temp[0]]; temp[1]=Sbox[temp[1]];
                temp[2]=Sbox[temp[2]]; temp[3]=Sbox[temp[3]];
                // Rcon
                temp[0] ^= Rcon[(i/8)-1];
            }else if(i%8==4){
                temp[0]=Sbox[temp[0]]; temp[1]=Sbox[temp[1]];
                temp[2]=Sbox[temp[2]]; temp[3]=Sbox[temp[3]];
            }
            int prev = 4*(i-8);
            rk[4*i+0] = rk[prev+0] ^ temp[0];
            rk[4*i+1] = rk[prev+1] ^ temp[1];
            rk[4*i+2] = rk[prev+2] ^ temp[2];
            rk[4*i+3] = rk[prev+3] ^ temp[3];
            ++i;
        }
    };

    auto aesEncryptBlock = [&](int *in, int *out, int *roundKeys){
        int state[16];
        int i=0;
        while(i<16){ state[i]=in[i]; ++i; }
        addRoundKey(state, roundKeys);
        int r=1;
        while(r<14){
            subBytes(state);
            shiftRows(state);
            // MixColumns
            int c=0;
            while(c<4){
                mixColumn(state+4*c);
                ++c;
            }
            addRoundKey(state, roundKeys+16*r);
            ++r;
        }
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, roundKeys+16*14);
        i=0;
        while(i<16){ out[i]=state[i]; ++i; }
    };

    auto xorBlock = [&](int *a, int *b, int *c){
        int i=0;
        while(i<16){ c[i]=a[i]^b[i]; ++i; }
    };

    auto dbl = [&](int *in, int *out){
        int carry = (in[0]>>7)&1;
        int i=0;
        while(i<15){
            out[i]=((in[i]<<1)&0xff) | ((in[i+1]>>7)&1);
            ++i;
        }
        out[15]=((in[15]<<1)&0xff) ^ (0x87*carry);
    };

    /*-------------------  Key schedule  -------------------*/
    int roundKey[240];               // 60 words * 4 bytes
    keyExpand(key, roundKey);

    /*-------------------  OCB encryption (simplified)  -------------------*/
    int offset[16];
    aesEncryptBlock(nonce, offset, roundKey);   // offset_0 = E(K, nonce)

    int checksum[16] = {0};
    int ciphertext[PLAINTEXT_SIZE];
    int blockCnt = PLAINTEXT_SIZE/16;
    int blk = 0;
    while(blk<blockCnt){
        int dblOff[16];
        dbl(offset, dblOff);
        for(int i=0;i<16;++i) offset[i]=dblOff[i];   // offset_i = 2*offset_{i-1}

        int tmp[16];
        xorBlock(plain+16*blk, offset, tmp);        // P_i xor offset_i
        int enc[16];
        aesEncryptBlock(tmp, enc, roundKey);       // E(K, …)
        xorBlock(enc, offset, ciphertext+16*blk);  // C_i

        xorBlock(checksum, plain+16*blk, checksum); // checksum ^= P_i
        ++blk;
    }

    // Tag = E(K, offset xor checksum)
    int tagIn[16];
    xorBlock(offset, checksum, tagIn);
    int tag[16];
    aesEncryptBlock(tagIn, tag, roundKey);

    /*-------------------  Output  -------------------*/
    std::cout << "Ciphertext: ";
    for(int i=0;i<PLAINTEXT_SIZE;++i){
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (ciphertext[i]&0xff);
    }
    std::cout << "\nTag:        ";
    for(int i=0;i<16;++i){
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (tag[i]&0xff);
    }
    std::cout << std::dec << std::endl;
    return 0;
}
