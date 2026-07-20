#include <iostream>
#include <iomanip>

/* LLM input variant 2: small-diverse */
int main() {
    /* ---------- heap‑allocated tables ---------- */
    int* sbox = new int[256];
    int* rcon = new int[11];
    /* S‑box values (standard AES) */
    int sb[256] = {
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
    for (int i=0;i<256;i++) sbox[i]=sb[i];
    /* Rcon values (only first 10 needed) */
    int rc[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};
    for (int i=0;i<11;i++) rcon[i]=rc[i];

    /* ---------- modular lambdas ---------- */
    auto subBytes = [&](int* st){
        for (int i=0;i<16;i++) st[i]=sbox[st[i]];
    };
    auto shiftRows = [&](int* st){
        int t;
        /* row 1 */
        t=st[1]; st[1]=st[5]; st[5]=st[9]; st[9]=st[13]; st[13]=t;
        /* row 2 */
        t=st[2]; st[2]=st[10]; st[10]=t;
        t=st[6]; st[6]=st[14]; st[14]=t;
        /* row 3 */
        t=st[3]; st[3]=st[15]; st[15]=st[11]; st[11]=st[7]; st[7]=t;
    };
    auto mixColumns = [&](int* st){
        for (int c=0;c<4;c++){
            int i0=4*c, i1=i0+1, i2=i0+2, i3=i0+3;
            int a0=st[i0], a1=st[i1], a2=st[i2], a3=st[i3];
            int r0 = ((a0<<1)&0xFF) ^ ((a0>>7)&0x1)*0x1B ^ a1 ^ a2 ^ a3;
            int r1 = a0 ^ ((a1<<1)&0xFF) ^ ((a1>>7)&0x1)*0x1B ^ a2 ^ a3;
            int r2 = a0 ^ a1 ^ ((a2<<1)&0xFF) ^ ((a2>>7)&0x1)*0x1B ^ a3;
            int r3 = a0 ^ a1 ^ a2 ^ ((a3<<1)&0xFF) ^ ((a3>>7)&0x1)*0x1B;
            st[i0]=r0&0xFF; st[i1]=r1&0xFF; st[i2]=r2&0xFF; st[i3]=r3&0xFF;
        }
    };
    auto addRoundKey = [&](int* st, int* rk){
        for (int i=0;i<16;i++) st[i]=(st[i]^rk[i])&0xFF;
    };
    auto rotWord = [&](int* w){
        int t=w[0];
        w[0]=w[1]; w[1]=w[2]; w[2]=w[3]; w[3]=t;
    };
    auto subWord = [&](int* w){
        for (int i=0;i<4;i++) w[i]=sbox[w[i]];
    };
    auto keyExp = [&](int* key, int* roundKeys){
        for (int i=0;i<16;i++) roundKeys[i]=key[i];
        int i=16, r=1;
        int temp[4];
        while (i<176){
            for (int j=0;j<4;j++) temp[j]=roundKeys[i-4+j];
            if (i%16==0){
                rotWord(temp);
                subWord(temp);
                temp[0]=(temp[0]^rcon[r])&0xFF;
                r++;
            }
            for (int j=0;j<4;j++){
                roundKeys[i]= (roundKeys[i-16]^temp[j])&0xFF;
                i++;
            }
        }
    };
    auto encryptBlock = [&](int* in, int* out, int* roundKeys){
        int state[16];
        for (int i=0;i<16;i++) state[i]=in[i];
        addRoundKey(state, roundKeys);
        int round=1;
        while (round<10){
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, roundKeys+16*round);
            round++;
        }
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, roundKeys+160);
        for (int i=0;i<16;i++) out[i]=state[i];
    };
    auto leftShiftOne = [&](int* src, int* dst){
        int carry=0;
        for (int i=15;i>=0;i--){
            int cur = ((src[i]<<1)&0xFF) | carry;
            dst[i]=cur;
            carry = (src[i]&0x80)?1:0;
        }
    };
    auto xor128 = [&](int* a, int* b, int* dst){
        for (int i=0;i<16;i++) dst[i]=(a[i]^b[i])&0xFF;
    };
    auto padBlock = [&](int* src, int len, int* dst){
        for (int i=0;i<16;i++) dst[i]=0;
        for (int i=0;i<len;i++) dst[i]=src[i];
        if (len<16){
            dst[len]=0x80;
            for (int i=len+1;i<16;i++) dst[i]=0;
        }
    };
    auto generateSubkeys = [&](int* key, int* K1, int* K2){
        int* roundKeys = new int[176];
        keyExp(key, roundKeys);
        int L[16]={0};
        encryptBlock(L, L, roundKeys);
        delete[] roundKeys;
        leftShiftOne(L, K1);
        if (L[0]&0x80) K1[15]^=0x87;
        leftShiftOne(K1, K2);
        if (K1[0]&0x80) K2[15]^=0x87;
    };

    /* ---------- test vectors (edge cases) ---------- */
    int key[16];
    for (int i=0;i<16;i++) key[i]= (i%2==0) ? (i*0x0F) & 0xFF : ((i*0x11)+0x0A) & 0xFF;  // varied key

    int messages[][64] = {
        {},                                    // zero‑length
        {0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,
         0x90,0xA0,0xB0,0xC0,0xD0,0xE0,0xF0,0x00}, // exactly one block
        {0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x11,0x22,
         0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00,
         0x01,0x02,0x03,0x04}                // 20 bytes (needs padding)
    };
    int msgLens[] = {0,16,20};

    for (int t=0;t<3;t++){
        int msgLen = msgLens[t];
        int* msg = new int[msgLen];
        for (int i=0;i<msgLen;i++) msg[i]=messages[t][i];

        /* ---------- CMAC computation ---------- */
        int K1[16], K2[16];
        generateSubkeys(key, K1, K2);

        int n = (msgLen+15)/16;               // number of blocks (ceil)
        if (n==0) n=1;                         // at least one block

        int* M_last = new int[16];
        int* M_i = new int[16];

        if (msgLen!=0 && (msgLen%16)==0){
            // last block is complete -> XOR with K1
            for (int i=0;i<16;i++) M_last[i]=msg[(n-1)*16 + i];
            xor128(M_last, K1, M_last);
        }else{
            // need padding -> XOR with K2
            int lastLen = msgLen%16;
            padBlock(msg+(n-1)*16, lastLen, M_last);
            xor128(M_last, K2, M_last);
        }

        int X[16];
        for (int i=0;i<16;i++) X[i]=0;

        int* roundKeys = new int[176];
        keyExp(key, roundKeys);

        for (int i=0;i<n-1;i++){
            for (int j=0;j<16;j++) M_i[j]=msg[i*16 + j];
            xor128(X, M_i, X);
            encryptBlock(X, X, roundKeys);
        }
        xor128(X, M_last, X);
        encryptBlock(X, X, roundKeys);

        /* ---------- output ---------- */
        std::cout << "Test " << t+1 << " (len=" << msgLen << "): ";
        for (int i=0;i<16;i++) std::cout << std::hex << std::setw(2) << std::setfill('0') << X[i];
        std::cout << std::dec << std::endl;

        delete[] msg;
        delete[] roundKeys;
        delete[] M_i;
        delete[] M_last;
    }

    delete[] sbox;
    delete[] rcon;
    return 0;
}
