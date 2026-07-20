#include <iostream>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */
int main()
{
    /* ---------- S‑box ---------- */
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

    /* ---------- Rcon ---------- */
    int Rcon[15] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
                    0x6C,0xD8,0xAB,0x4D,0x9A};

    /* ---------- Helper lambdas ---------- */
    auto xtime = [&](int x)->int{
        int r = (x << 1) & 0xFF;
        r ^= ((x >> 7) & 1) ? 0x1B : 0;
        return r & 0xFF;
    };

    auto subBytes = [&](int state[4][4]){
        int i=0,j=0;
        while(i<4){
            j=0;
            while(j<4){
                state[i][j] = sbox[state[i][j]];
                j=j+1;
            }
            i=i+1;
        }
    };

    auto shiftRows = [&](int state[4][4]){
        int tmp;

        /* row 1 left 1 */
        tmp = state[1][0];
        state[1][0]=state[1][1];
        state[1][1]=state[1][2];
        state[1][2]=state[1][3];
        state[1][3]=tmp;

        /* row 2 left 2 */
        tmp = state[2][0];
        state[2][0]=state[2][2];
        state[2][2]=tmp;
        tmp = state[2][1];
        state[2][1]=state[2][3];
        state[2][3]=tmp;

        /* row 3 left 3 (right 1) */
        tmp = state[3][3];
        state[3][3]=state[3][2];
        state[3][2]=state[3][1];
        state[3][1]=state[3][0];
        state[3][0]=tmp;
    };

    auto mixColumns = [&](int state[4][4]){
        int c=0;
        while(c<4){
            int a0=state[0][c];
            int a1=state[1][c];
            int a2=state[2][c];
            int a3=state[3][c];

            int t = a0 ^ a1 ^ a2 ^ a3;
            int u = a0;

            state[0][c] = a0 ^ t ^ xtime(a0 ^ a1);
            state[1][c] = a1 ^ t ^ xtime(a1 ^ a2);
            state[2][c] = a2 ^ t ^ xtime(a2 ^ a3);
            state[3][c] = a3 ^ t ^ xtime(a3 ^ u);

            c=c+1;
        }
    };

    auto addRoundKey = [&](int state[4][4], int roundKey[4][4]){
        int i=0,j=0;
        while(i<4){
            j=0;
            while(j<4){
                state[i][j] ^= roundKey[i][j];
                j=j+1;
            }
            i=i+1;
        }
    };

    /* ---------- Key expansion ---------- */
    auto expandKey = [&](int key[32], int roundKey[15][4][4]){
        int w[60][4];               // 60 words, each 4 bytes
        int i=0;
        while(i<8){
            w[i][0]=key[4*i];
            w[i][1]=key[4*i+1];
            w[i][2]=key[4*i+2];
            w[i][3]=key[4*i+3];
            i=i+1;
        }

        i=8;
        while(i<60){
            int temp[4];
            temp[0]=w[i-1][0];
            temp[1]=w[i-1][1];
            temp[2]=w[i-1][2];
            temp[3]=w[i-1][3];

            if(i%8==0){
                /* RotWord */
                int t0=temp[0];
                temp[0]=temp[1];
                temp[1]=temp[2];
                temp[2]=temp[3];
                temp[3]=t0;
                /* SubWord */
                temp[0]=sbox[temp[0]];
                temp[1]=sbox[temp[1]];
                temp[2]=sbox[temp[2]];
                temp[3]=sbox[temp[3]];
                /* Rcon */
                temp[0] ^= Rcon[i/8-1];
            }else if(i%8==4){
                temp[0]=sbox[temp[0]];
                temp[1]=sbox[temp[1]];
                temp[2]=sbox[temp[2]];
                temp[3]=sbox[temp[3]];
            }

            w[i][0]=w[i-8][0] ^ temp[0];
            w[i][1]=w[i-8][1] ^ temp[1];
            w[i][2]=w[i-8][2] ^ temp[2];
            w[i][3]=w[i-8][3] ^ temp[3];
            i=i+1;
        }

        /* copy to roundKey matrix */
        int r=0,c=0;
        while(r<15){
            c=0;
            while(c<4){
                roundKey[r][0][c]=w[4*r + c][0];
                roundKey[r][1][c]=w[4*r + c][1];
                roundKey[r][2][c]=w[4*r + c][2];
                roundKey[r][3][c]=w[4*r + c][3];
                c=c+1;
            }
            r=r+1;
        }
    };

    /* ---------- AES block encrypt (ECB) ---------- */
    auto encryptBlock = [&](int in[16], int out[16], int roundKey[15][4][4]){
        int state[4][4];
        int i=0;
        while(i<16){
            state[i%4][i/4]=in[i] & 0xFF;
            i=i+1;
        }

        addRoundKey(state, roundKey[0]);

        int round=1;
        while(round<14){
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, roundKey[round]);
            round=round+1;
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, roundKey[14]);

        i=0;
        while(i<16){
            out[i]=state[i%4][i/4] & 0xFF;
            i=i+1;
        }
    };

    /* ---------- OFB mode ---------- */
    auto ofbEncrypt = [&](int pt[], int ptLen, int key[32], int iv[16], int ct[]){
        int roundKey[15][4][4];
        expandKey(key, roundKey);

        int feedback[16];
        int j=0;
        while(j<16){
            feedback[j]=iv[j] & 0xFF;
            j=j+1;
        }

        int blockCnt = ptLen / 16;
        int blockIdx = 0;
        while(blockIdx < blockCnt){
            int keystream[16];
            encryptBlock(feedback, keystream, roundKey);

            int k=0;
            while(k<16){
                ct[blockIdx*16 + k] = (pt[blockIdx*16 + k] ^ keystream[k]) & 0xFF;
                k=k+1;
            }

            /* feedback for next round is the keystream (OFB) */
            j=0;
            while(j<16){
                feedback[j]=keystream[j];
                j=j+1;
            }

            blockIdx=blockIdx+1;
        }
    };

    /* ---------- Test vectors (deterministic, reverse‑ordered) ---------- */
    int key[32];
    int iv[16];
    int plaintext[16];
    int ciphertext[16];

    int i=0;
    while(i<32){
        key[i]=0xFF - i;                // 0xFF .. 0xE0 descending
        i=i+1;
    }
    i=0;
    while(i<16){
        iv[i]=0xFF - i;                 // 0xFF .. 0xF0 descending
        i=i+1;
    }
    i=0;
    while(i<16){
        plaintext[i]=0xFF - i;          // 0xFF .. 0xF0 descending
        i=i+1;
    }

    /* ---------- Perform OFB encryption ---------- */
    ofbEncrypt(plaintext, 16, key, iv, ciphertext);

    /* ---------- Output ciphertext as hex ---------- */
    std::cout << "Ciphertext (hex): ";
    i=0;
    while(i<16){
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (ciphertext[i] & 0xFF);
        i=i+1;
    }
    std::cout << std::dec << std::endl;

    return 0;
}
