/********************************************************************
 * ARIA-CBC implementation (version #1)
 * ---------------------------------------------------------------
 *  * Only signed int is used (no unsigned, double, long, const)
 *  * Verbose step‑by‑step operations
 *  * Fused expressions where possible
 *  * std::vector is used for all dynamic buffers
 *  * Class based design
 *  * Deterministic test vectors are generated inside main()
 *
 *  This program encrypts a two‑block plaintext with a 128‑bit key
 *  in CBC mode and prints the resulting ciphertext in hexadecimal.
 ********************************************************************/
/* LLM input variant 4: signed-extremes */

#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>

/* ------------------------------------------------------------------
 *  S‑boxes (values are signed int, range 0‑255)
 * ------------------------------------------------------------------ */
static int S1[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

static int S2[256] = {
    0xe2,0x4e,0x54,0xfc,0x94,0xc2,0x4a,0xcc,0x62,0x0d,0x6a,0x46,0x3c,0x4d,0x8b,0xd1,
    0x5e,0xfa,0x64,0xcb,0xb4,0x97,0xbe,0x2b,0xbc,0x0a,0x4f,0xa6,0x5c,0x12,0x2c,0x69,
    0xe5,0x89,0x58,0x5a,0x30,0x2e,0x44,0x24,0x0b,0xe1,0x76,0x5d,0xd6,0x1f,0x34,0x1c,
    0xf0,0x71,0x0a,0x3b,0xd6,0x58,0x51,0x79,0x2a,0x9c,0x5b,0x5f,0x7b,0x44,0x63,0x3c,
    0x4b,0x49,0x94,0x7e,0x6c,0x7c,0x33,0x68,0x6b,0xa8,0x70,0x8f,0x41,0x3c,0xb9,0x7d,
    0x00,0x57,0x0b,0x5e,0xd6,0x5b,0x0e,0xf4,0x5e,0xc5,0xf0,0x00,0x07,0x36,0x0f,0x42,
    0x89,0x7a,0xa5,0x68,0x2a,0x0f,0x5b,0x5b,0x9c,0x00,0x4d,0x00,0x5c,0x12,0x74,0x8d,
    0x4d,0x77,0x73,0x1e,0xcc,0x1f,0x5f,0x69,0x22,0x15,0x45,0x6a,0x73,0x00,0x19,0xdb,
    0x13,0x5e,0x2c,0x40,0x2c,0x9b,0x06,0x7c,0x9c,0x71,0x91,0x5c,0xf5,0x5d,0x91,0x71,
    0x16,0x01,0x21,0x5c,0x5e,0x68,0x2c,0x0c,0x0c,0x56,0x4c,0x0c,0x71,0x33,0x1a,0x5a,
    0x2b,0x5c,0x2e,0x0f,0x4d,0x5b,0x3c,0x0a,0x4c,0x5c,0x2c,0x6c,0x05,0x5c,0x58,0x5c,
    0x00,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
    0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
    0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
    0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
    0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c
};

/* ------------------------------------------------------------------
 *  ARIA class – key schedule, block encrypt/decrypt, CBC mode
 * ------------------------------------------------------------------ */
class ARIA {
public:
    ARIA(const std::vector<int>& key) {
        /* key must be 16 bytes (128‑bit) for this demo */
        keySchedule(key);
    }

    /* encrypt a vector of plaintext bytes (multiple of 16) in CBC mode */
    std::vector<int> encryptCBC(const std::vector<int>& plain,
                                const std::vector<int>& iv) {
        std::vector<int> out(plain.size());
        std::vector<int> prevBlock = iv;                // initial chaining value
        for (int i = 0; i < (int)plain.size(); i += 16) {
            std::vector<int> block(16);
            for (int j = 0; j < 16; ++j) block[j] = plain[i + j] ^ prevBlock[j];
            std::vector<int> encBlock = encryptBlock(block);
            for (int j = 0; j < 16; ++j) out[i + j] = encBlock[j];
            prevBlock = encBlock;                       // update chaining value
        }
        return out;
    }

private:
    std::vector< std::vector<int> > roundKey;   // each sub‑key = 16 bytes

    void keySchedule(const std::vector<int>& master) {
        std::vector<int> w0(master.begin(), master.begin() + 4);
        std::vector<int> w1(master.begin() + 4, master.begin() + 8);
        std::vector<int> w2(master.begin() + 8, master.begin() + 12);
        std::vector<int> w3(master.begin() + 12, master.begin() + 16);

        auto sub1 = [&](std::vector<int>& in) {
            for (int i = 0; i < 4; ++i) {
                int a = in[i];
                int b = (a >> 24) & 0xff;
                int c = (a >> 16) & 0xff;
                int d = (a >> 8 ) & 0xff;
                int e = a & 0xff;
                int nb = (S1[b] << 24) | (S2[c] << 16) | (S1[d] << 8) | S2[e];
                in[i] = nb;
            }
        };

        auto sub2 = [&](std::vector<int>& in) {
            for (int i = 0; i < 4; ++i) {
                int a = in[i];
                int b = (a >> 24) & 0xff;
                int c = (a >> 16) & 0xff;
                int d = (a >> 8 ) & 0xff;
                int e = a & 0xff;
                int nb = (S2[b] << 24) | (S1[c] << 16) | (S2[d] << 8) | S1[e];
                in[i] = nb;
            }
        };

        auto diffusion = [&](std::vector<int>& in) {
            const int M[4][4] = {
                {0x01,0x03,0x01,0x03},
                {0x03,0x01,0x03,0x01},
                {0x01,0x03,0x01,0x03},
                {0x03,0x01,0x03,0x01}
            };
            std::vector<int> out(4,0);
            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 4; ++c) {
                    int src = in[c];
                    int b0 = (src >> 24) & 0xff;
                    int b1 = (src >> 16) & 0xff;
                    int b2 = (src >> 8 ) & 0xff;
                    int b3 = src & 0xff;
                    int m = M[r][c];
                    int mult = (m == 1) ? (b0<<24|b1<<16|b2<<8|b3) :
                               ((b0<<24|b1<<16|b2<<8|b3) ^ ((b0<<24|b1<<16|b2<<8|b3) << 1));
                    out[r] ^= mult;
                }
            }
            in = out;
        };

        std::vector<int> t0 = w0; sub1(t0); diffusion(t0);
        std::vector<int> t1 = w1; sub2(t1); diffusion(t1);
        std::vector<int> t2 = w2; sub1(t2); diffusion(t2);
        std::vector<int> t3 = w3; sub2(t3); diffusion(t3);

        roundKey.push_back({w0[0],w0[1],w0[2],w0[3]});            // WK0
        roundKey.push_back({t0[0]^w1[0],t0[1]^w1[1],t0[2]^w1[2],t0[3]^w1[3]}); // K1
        roundKey.push_back({t1[0]^w2[0],t1[1]^w2[1],t1[2]^w2[2],t1[3]^w2[3]}); // K2
        roundKey.push_back({t2[0]^w3[0],t2[1]^w3[1],t2[2]^w3[2],t2[3]^w3[3]}); // K3

        std::vector<int> wk = {w0[0],w0[1],w0[2],w0[3]};
        for (int r = 4; r <= 13; ++r) {
            std::vector<int> rot(4);
            for (int i = 0; i < 4; ++i) {
                int src = wk[i];
                rot[i] = ((src << 8) | ((src >> 24) & 0xff)) & 0xffffffff;
            }
            if (r % 2 == 0) sub1(rot); else sub2(rot);
            for (int i = 0; i < 4; ++i) rot[i] ^= r;
            roundKey.push_back(rot);
            wk = rot;
        }

        roundKey.push_back({w3[0],w3[1],w3[2],w3[3]});            // WK1
    }

    std::vector<int> encryptBlock(const std::vector<int>& inBlock) {
        std::vector<int> state(4);
        for (int i = 0; i < 4; ++i) {
            state[i] = (inBlock[4*i] << 24) | (inBlock[4*i+1] << 16) |
                       (inBlock[4*i+2] << 8) | inBlock[4*i+3];
        }

        for (int i = 0; i < 4; ++i) state[i] ^= roundKey[0][i];

        for (int r = 1; r <= 12; ++r) {
            if (r % 3 == 1) substituteType1(state);
            else if (r % 3 == 2) substituteType2(state);
            else substituteType3(state);

            diffusion(state);

            for (int i = 0; i < 4; ++i) state[i] ^= roundKey[r][i];
        }

        for (int i = 0; i < 4; ++i) state[i] ^= roundKey[14][i];

        std::vector<int> out(16);
        for (int i = 0; i < 4; ++i) {
            out[4*i]   = (state[i] >> 24) & 0xff;
            out[4*i+1] = (state[i] >> 16) & 0xff;
            out[4*i+2] = (state[i] >> 8 ) & 0xff;
            out[4*i+3] = state[i] & 0xff;
        }
        return out;
    }

    void substituteType1(std::vector<int>& w) {
        for (int i = 0; i < 4; ++i) {
            int a = w[i];
            int b = (a >> 24) & 0xff;
            int c = (a >> 16) & 0xff;
            int d = (a >> 8 ) & 0xff;
            int e = a & 0xff;
            w[i] = (S1[b] << 24) | (S2[c] << 16) | (S1[d] << 8) | S2[e];
        }
    }

    void substituteType2(std::vector<int>& w) {
        for (int i = 0; i < 4; ++i) {
            int a = w[i];
            int b = (a >> 24) & 0xff;
            int c = (a >> 16) & 0xff;
            int d = (a >> 8 ) & 0xff;
            int e = a & 0xff;
            w[i] = (S2[b] << 24) | (S1[c] << 16) | (S2[d] << 8) | S1[e];
        }
    }

    void substituteType3(std::vector<int>& w) {
        for (int i = 0; i < 4; ++i) {
            int a = w[i];
            int b = (a >> 24) & 0xff;
            int c = (a >> 16) & 0xff;
            int d = (a >> 8 ) & 0xff;
            int e = a & 0xff;
            w[i] = (S1[b] << 24) | (S1[c] << 16) | (S2[d] << 8) | S2[e];
        }
    }

    void diffusion(std::vector<int>& w) {
        const int M[4][4] = {
            {0x01,0x03,0x01,0x03},
            {0x03,0x01,0x03,0x01},
            {0x01,0x03,0x01,0x03},
            {0x03,0x01,0x03,0x01}
        };
        std::vector<int> out(4,0);
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                int src = w[c];
                int b0 = (src >> 24) & 0xff;
                int b1 = (src >> 16) & 0xff;
                int b2 = (src >> 8 ) & 0xff;
                int b3 = src & 0xff;
                int m = M[r][c];
                int mult = (m == 1) ? (b0<<24|b1<<16|b2<<8|b3) :
                           ((b0<<24|b1<<16|b2<<8|b3) ^ ((b0<<24|b1<<16|b2<<8|b3) << 1));
                out[r] ^= mult;
            }
        }
        w = out;
    }
};

/* --------------------------------------------------------------------
 *  Helper: convert a byte vector to printable hexadecimal string
 * -------------------------------------------------------------------- */
std::string hexString(const std::vector<int>& data) {
    std::ostringstream oss;
    for (size_t i = 0; i < data.size(); ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (data[i] & 0xff);
        if ((i + 1) % 16 == 0) oss << "\n";
        else oss << " ";
    }
    return oss.str();
}

/* --------------------------------------------------------------------
 *  Main – deterministic test vectors (mixed zeros and positives)
 * -------------------------------------------------------------------- */
int main() {
    /* 128‑bit key: mix of zeroes and small positive numbers */
    std::vector<int> key = {
        0, 0, 0, 0,
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12
    };

    /* Plaintext: two blocks with zeros and ascending values */
    std::vector<int> plain(32);
    for (int i = 0; i < 32; ++i) {
        plain[i] = (i < 8) ? 0 : i;   // first eight bytes are zero, rest ascend
    }

    /* IV: all zeroes */
    std::vector<int> iv(16, 0);

    ARIA aria(key);
    std::vector<int> cipher = aria.encryptCBC(plain, iv);

    std::cout << "Key (hex):\n" << hexString(key) << "\n";
    std::cout << "Plaintext (hex):\n" << hexString(plain) << "\n";
    std::cout << "IV (hex):\n" << hexString(iv) << "\n";
    std::cout << "Ciphertext (hex):\n" << hexString(cipher) << "\n";

    return 0;
}
