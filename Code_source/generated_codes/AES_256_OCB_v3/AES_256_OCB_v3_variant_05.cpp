#include <iostream>
#include <iomanip>

int main()
{
    /* LLM input variant 5: duplicate-heavy */
    /*-------------------  AES‑256 tables  -------------------*/
    int Sbox[256] = {
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
        99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99
    };
    int Rcon[15] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

    /*-------------------  Test vectors  -------------------*/
    int key[32] = {
        0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
        0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
        0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
        0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f
    };
    int nonce[16] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    int plain[32] = {
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
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
