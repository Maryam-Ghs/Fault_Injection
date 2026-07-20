#include <iostream>
#include <iomanip>

/* LLM input variant 3: zeros-and-ones */

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
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01
    };
    int nonce[16] = {0};
    int plain[32] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
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
    int ciphertext[32];
    int blockCnt = 32/16;
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
    for(int i=0;i<32;++i){
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (ciphertext[i]&0xff);
    }
    std::cout << "\nTag:        ";
    for(int i=0;i<16;++i){
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (tag[i]&0xff);
    }
    std::cout << std::dec << std::endl;
    return 0;
}
