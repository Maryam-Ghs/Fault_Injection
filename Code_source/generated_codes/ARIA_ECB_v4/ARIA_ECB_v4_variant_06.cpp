/* LLM input variant 6: ordered-structured */
#include <iostream>
#include <vector>
#include <iomanip>

static int SB1[256] = {
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

static int SB2[256];
static int SB3[256];
static int SB4[256];

void init_sboxes()
{
    for (int i = 0; i < 256; ++i) {
        SB2[i] = SB1[(i +  85) & 0xFF];
        SB3[i] = SB1[(i + 170) & 0xFF];
        SB4[i] = SB1[(i + 255) & 0xFF];
    }
}

void addRoundKey(std::vector<int> &st, const std::vector<int> &rk)
{
    for (int i = 0; i < 16; ++i) st[i] ^= rk[i];
}

void subLayer(std::vector<int> &st, int odd)
{
    if (odd) {
        st[ 0] = SB1[st[ 0]]; st[ 1] = SB2[st[ 1]];
        st[ 2] = SB3[st[ 2]]; st[ 3] = SB4[st[ 3]];
        st[ 4] = SB1[st[ 4]]; st[ 5] = SB2[st[ 5]];
        st[ 6] = SB3[st[ 6]]; st[ 7] = SB4[st[ 7]];
        st[ 8] = SB1[st[ 8]]; st[ 9] = SB2[st[ 9]];
        st[10] = SB3[st[10]]; st[11] = SB4[st[11]];
        st[12] = SB1[st[12]]; st[13] = SB2[st[13]];
        st[14] = SB3[st[14]]; st[15] = SB4[st[15]];
    } else {
        st[ 0] = SB2[st[ 0]]; st[ 1] = SB3[st[ 1]];
        st[ 2] = SB4[st[ 2]]; st[ 3] = SB1[st[ 3]];
        st[ 4] = SB2[st[ 4]]; st[ 5] = SB3[st[ 5]];
        st[ 6] = SB4[st[ 6]]; st[ 7] = SB1[st[ 7]];
        st[ 8] = SB2[st[ 8]]; st[ 9] = SB3[st[ 9]];
        st[10] = SB4[st[10]]; st[11] = SB1[st[11]];
        st[12] = SB2[st[12]]; st[13] = SB3[st[13]];
        st[14] = SB4[st[14]]; st[15] = SB1[st[15]];
    }
}

void diffusion(std::vector<int> &st)
{
    int a0, a1, a2, a3;
    a0 = st[0]; a1 = st[1]; a2 = st[2]; a3 = st[3];
    st[0] = a0 ^ a1 ^ a2 ^ a3;
    st[1] = a0 ^ a1;
    st[2] = a1 ^ a2;
    st[3] = a2 ^ a3;

    a0 = st[4]; a1 = st[5]; a2 = st[6]; a3 = st[7];
    st[4] = a0 ^ a1 ^ a2 ^ a3;
    st[5] = a0 ^ a1;
    st[6] = a1 ^ a2;
    st[7] = a2 ^ a3;

    a0 = st[8]; a1 = st[9]; a2 = st[10]; a3 = st[11];
    st[8]  = a0 ^ a1 ^ a2 ^ a3;
    st[9]  = a0 ^ a1;
    st[10] = a1 ^ a2;
    st[11] = a2 ^ a3;

    a0 = st[12]; a1 = st[13]; a2 = st[14]; a3 = st[15];
    st[12] = a0 ^ a1 ^ a2 ^ a3;
    st[13] = a0 ^ a1;
    st[14] = a1 ^ a2;
    st[15] = a2 ^ a3;
}

std::vector<int> rotateKey(const std::vector<int> &k, int shift)
{
    std::vector<int> r(16);
    for (int i = 0; i < 16; ++i) {
        int src = (i + shift) & 0x0F;
        r[i] = k[src];
    }
    return r;
}

void keySchedule(const std::vector<int> &master,
                 std::vector< std::vector<int> > &rks)
{
    for (int r = 0; r <= 12; ++r) {
        rks[r] = rotateKey(master, r);
    }
}

void encryptBlock(std::vector<int> &blk,
                  const std::vector< std::vector<int> > &rks)
{
    addRoundKey(blk, rks[0]);
    for (int round = 1; round <= 12; ++round) {
        subLayer(blk, (round & 1));
        diffusion(blk);
        addRoundKey(blk, rks[round]);
    }
}

void printBlock(const std::vector<int> &blk)
{
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (blk[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;
}

int main()
{
    init_sboxes();

    /* ordered‑structured key: ascending values */
    std::vector<int> key = {
        0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F
    };

    /* ordered‑structured plaintext: three blocks in ascending order */
    std::vector< std::vector<int> > plain = {
        { 0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
          0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F },
        { 0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17,
          0x18,0x19,0x1A,0x1B, 0x1C,0x1D,0x1E,0x1F },
        { 0x20,0x21,0x22,0x23, 0x24,0x25,0x26,0x27,
          0x28,0x29,0x2A,0x2B, 0x2C,0x2D,0x2E,0x2F }
    };

    std::vector< std::vector<int> > rkeys(13, std::vector<int>(16));
    keySchedule(key, rkeys);

    std::cout << "Ciphertext (ECB, ARIA‑like):" << std::endl;
    for (size_t b = 0; b < plain.size(); ++b) {
        std::vector<int> block = plain[b];
        encryptBlock(block, rkeys);
        printBlock(block);
    }
    return 0;
}
