#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 4: signed-extremes */
int main() {
    int sbA[256], sbB[256];
    int i=0;
    while(i<256){ sbA[i]=0; sbB[i]=0; ++i; }
    int aesS[256] = { /* original values omitted for brevity */ 0 };
    int aesInv[256] = { /* original values omitted for brevity */ 0 };
    i=0;
    while(i<256){ sbA[i]=aesS[i]; sbB[i]=aesInv[i]; ++i; }

    auto mul2=[](int x){int y=(x<<1);int mask=(x&0x80)?0x1b:0x00;return (y^mask)&0xff;};
    auto mul3=[&](int x){return (mul2(x)^x)&0xff;};

    auto subBytes=[&](const std::vector<int>& src,int typ){
        std::vector<int> dst(16);
        for(int idx=0; idx<16; ++idx){
            int b=src[idx];
            dst[idx]=(typ==0)?((idx%2==0)?sbA[b]:sbB[b]):((idx%2==0)?sbB[b]:sbA[b]);
        }
        return dst;
    };

    auto diffuse=[&](const std::vector<int>& src,int typ){
        std::vector<int> dst(16);
        for(int col=0; col<4; ++col){
            int r0=src[0+col*4];
            int r1=src[1+col*4];
            int r2=src[2+col*4];
            int r3=src[3+col*4];
            int a0,a1,a2,a3;
            if(typ==0){
                a0=(mul2(r0)^mul3(r1)^r2^r3)&0xff;
                a1=(r0^mul2(r1)^mul3(r2)^r3)&0xff;
                a2=(r0^r1^mul2(r2)^mul3(r3))&0xff;
                a3=(mul3(r0)^r1^r2^mul2(r3))&0xff;
            }else{
                a0=(r0^r1^mul3(r2)^mul2(r3))&0xff;
                a1=(mul2(r0)^r1^r2^mul3(r3))&0xff;
                a2=(mul3(r0)^mul2(r1)^r2^r3)&0xff;
                a3=(r0^mul3(r1)^mul2(r2)^r3)&0xff;
            }
            dst[0+col*4]=a0; dst[1+col*4]=a1; dst[2+col*4]=a2; dst[3+col*4]=a3;
        }
        return dst;
    };

    auto genRoundKeys=[&](const std::vector<int>& master){
        std::vector<std::vector<int>> rks(13,std::vector<int>(16));
        for(int round=0; round<13; ++round){
            for(int pos=0; pos<16; ++pos){
                int srcIdx=(pos+round*3)%16;
                rks[round][pos]=master[srcIdx];
            }
        }
        return rks;
    };

    auto xorBlock=[&](const std::vector<int>& a,const std::vector<int>& b){
        std::vector<int> out(16);
        for(int idx=0; idx<16; ++idx) out[idx]=(a[idx]^b[idx])&0xff;
        return out;
    };

    auto encryptBlock=[&](const std::vector<int>& plain,const std::vector<std::vector<int>>& rks){
        std::vector<int> state=xorBlock(plain,rks[0]);
        for(int round=1; round<13; ++round){
            if(round%2==1){ state=subBytes(state,0); state=diffuse(state,0); }
            else{ state=subBytes(state,1); state=diffuse(state,1); }
            state=xorBlock(state,rks[round]);
        }
        return state;
    };

    std::vector<int> masterKey = { -128, -64, -1, 0, 1, 63, 127, -32,
                                  2, -2, 3, -3, 4, -4, 5, -5 };
    std::vector<int> plainBlock = { -127, -63, -31, -15, -7, -3, -1, 0,
                                    1, 3, 7, 15, 31, 63, 127, -128 };

    std::vector<std::vector<int>> roundKeys = genRoundKeys(masterKey);
    std::vector<int> cipherBlock = encryptBlock(plainBlock, roundKeys);

    std::cout << "Key       : ";
    for(int v:masterKey) std::cout << std::hex << std::setw(2) << std::setfill('0') << (v & 0xff);
    std::cout << std::dec << "\nPlaintext : ";
    for(int v:plainBlock) std::cout << std::hex << std::setw(2) << std::setfill('0') << (v & 0xff);
    std::cout << std::dec << "\nCiphertext: ";
    for(int v:cipherBlock) std::cout << std::hex << std::setw(2) << std::setfill('0') << v;
    std::cout << std::dec << std::endl;
    return 0;
}
