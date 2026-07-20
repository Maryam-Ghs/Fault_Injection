#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 4: signed-extremes */

/*=====================================================================
  ARIA ECB implementation (version #1)
=====================================================================*/

class ARIA_ECB {
public:
    int totalRounds;
    vector< vector<int> > roundKeys;
    ARIA_ECB(const vector<int>& masterKey){ totalRounds=12; expandKey(masterKey); }
    void encryptECB(const vector< vector<int> >& plain,
                    vector< vector<int> >& cipher){
        int idx=0;
        while(idx<(int)plain.size()){
            vector<int> outBlock(16);
            encryptBlock(plain[idx],outBlock);
            cipher.push_back(outBlock);
            idx=idx+1;
        }
    }
private:
    int S1[256]={108,45,53,61,35,24,16,78,44,88,170,86,112,128,93,98,
        31,149,84,131,103,137,30,100,71,90,60,115,141,209,172,111,
        172,15,152,38,115,79,152,112,31,83,150,119,81,166,38,17,
        119,71,156,114,69,115,70,110,126,167,100,24,96,59,0,101};
    int S2[256]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    void diffusionM0(const vector<int>& in, vector<int>& out){
        int a0=in[0]^in[4]^in[8]^in[12];
        int a1=in[1]^in[5]^in[9]^in[13];
        int a2=in[2]^in[6]^in[10]^in[14];
        int a3=in[3]^in[7]^in[11]^in[15];
        out[0]=out[4]=out[8]=out[12]=a0;
        out[1]=out[5]=out[9]=out[13]=a1;
        out[2]=out[6]=out[10]=out[14]=a2;
        out[3]=out[7]=out[11]=out[15]=a3;
    }
    void substitute(const vector<int>& src, vector<int>& dst, int round){
        int i=0;
        while(i<16){
            int v=src[i];
            dst[i]=(round&1)?S2[v]:S1[v];
            i=i+1;
        }
    }
    void expandKey(const vector<int>& master){
        roundKeys.push_back(master);
        int rc=0x1F;
        int r=0;
        while(r<totalRounds){
            vector<int> prev=roundKeys[r];
            vector<int> nxt(16);
            int j=0;
            while(j<16){
                nxt[j]=prev[j]^rc;
                j=j+1;
            }
            roundKeys.push_back(nxt);
            rc=(rc<<1)&0xFF;
            r=r+1;
        }
    }
    void encryptBlock(const vector<int>& plain, vector<int>& cipher){
        vector<int> state(16);
        int i=0;
        while(i<16){
            state[i]=plain[i]^roundKeys[0][i];
            i=i+1;
        }
        int round=1;
        while(round<totalRounds){
            vector<int> sub(16), dif(16);
            substitute(state,sub,round);
            diffusionM0(sub,dif);
            int k=0;
            while(k<16){
                state[k]=dif[k]^roundKeys[round][k];
                k=k+1;
            }
            round=round+1;
        }
        vector<int> finalSub(16);
        substitute(state,finalSub,totalRounds);
        int t=0;
        while(t<16){
            cipher[t]=finalSub[t]^roundKeys[totalRounds][t];
            t=t+1;
        }
    }
};

void printBlock(const vector<int>& blk){
    int i=0;
    while(i<16){
        int hi=(blk[i]>>4)&0xF, lo=blk[i]&0xF;
        char h=(hi<10)?('0'+hi):('A'+hi-10);
        char l=(lo<10)?('0'+lo):('A'+lo-10);
        cout<<h<<l<<' ';
        i=i+1;
    }
    cout<<endl;
}

int main() {
    // Key mixing signed extreme-like values (0, 127, 255, 64, 192, etc.)
    vector<int> key = {0, 127, 255, 64, 192, 32, 96, 160,
                       1, 126, 254, 65, 191, 33, 97, 159};

    ARIA_ECB cipherEngine(key);

    // Two plaintext blocks with a mix of low, middle, high byte values
    vector< vector<int> > plainBlocks = {
        {0, 50, 100, 150, 200, 250, 25, 75,
         125, 175, 225, 255, 10, 60, 110, 160},
        {255, 200, 150, 100, 50, 0, 230, 180,
         130, 80, 30, 5, 55, 105, 155, 205}
    };

    vector< vector<int> > cipherBlocks;
    cipherEngine.encryptECB(plainBlocks,cipherBlocks);

    cout<<"Key:"<<endl;
    printBlock(key);
    cout<<"\nPlaintext blocks:"<<endl;
    for(size_t i=0;i<plainBlocks.size();++i){
        cout<<"P["<<i<<"]: ";
        printBlock(plainBlocks[i]);
    }
    cout<<"\nCiphertext blocks (ECB):"<<endl;
    for(size_t i=0;i<cipherBlocks.size();++i){
        cout<<"C["<<i<<"]: ";
        printBlock(cipherBlocks[i]);
    }
    return 0;
}
