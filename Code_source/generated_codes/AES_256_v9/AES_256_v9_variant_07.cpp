/ * LLM input variant 7: reverse-adversarial * /
 // AES‑256 implementation – version #9
 // ------------------------------------------------------------
 // Restrictions applied:
 //   * Only int (no unsigned, long, double, const)
 //   * Stack‑allocated arrays only
 //   * No external input (deterministic test vector)
 //   * Class‑based design, simple readable style
 // ------------------------------------------------------------

 #include <iostream>
 #include <iomanip>

 class Aes256 {
 public:
     // Encrypt a single 16‑byte block (plaintext) with a 32‑byte key.
     // Returns ciphertext in the same array (in‑place).
     void encrypt(int pt[16], const int key[32]) {
         int roundKey[240];                     // 60 words * 4 bytes
         expandKey(key, roundKey);

         int state[16];
         for (int i = 0; i < 16; ++i) state[i] = pt[i] ^ roundKey[i];

         int round = 1;
         while (round <= 13) {
             subBytes(state);
             shiftRows(state);
             mixColumns(state);
             addRoundKey(state, roundKey + round * 16);
             ++round;
         }

         subBytes(state);
         shiftRows(state);
         addRoundKey(state, roundKey + round * 16);   // final round key

         for (int i = 0; i < 16; ++i) pt[i] = state[i];
     }

 private:
     // -----------------------------------------------------------------
     // Fixed tables (S‑box and Rcon)
     // -----------------------------------------------------------------
     static int sBox[256];
     static int rCon[15];

     // -----------------------------------------------------------------
     // Key expansion for 256‑bit key (produces 60 words = 240 bytes)
     // -----------------------------------------------------------------
     void expandKey(const int key[32], int out[240]) {
         // first 8 words are the key itself
         for (int i = 0; i < 32; ++i) out[i] = key[i];

         int i = 32;
         int r = 1;
         while (i < 240) {
             int temp[4];
             // copy previous word
             for (int j = 0; j < 4; ++j) temp[j] = out[i - 4 + j];

             if ((i / 4) % 8 == 0) {
                 rotWord(temp);
                 subWord(temp);
                 temp[0] ^= rCon[r - 1];
                 ++r;
             } else if ((i / 4) % 8 == 4) {
                 subWord(temp);
             }

             // XOR with word 8 positions earlier
             for (int j = 0; j < 4; ++j) {
                 out[i] = out[i - 32] ^ temp[j];
                 ++i;
             }
         }
     }

     // -----------------------------------------------------------------
     // Core AES transformations
     // -----------------------------------------------------------------
     void addRoundKey(int st[16], const int *rk) {
         for (int i = 0; i < 16; ++i) st[i] ^= rk[i];
     }

     void subBytes(int st[16]) {
         for (int i = 0; i < 16; ++i) st[i] = sBox[st[i]];
     }

     void shiftRows(int st[16]) {
         int tmp[16];
         // row 0 (no shift)
         tmp[0]  = st[0];  tmp[4]  = st[4];  tmp[8]  = st[8];  tmp[12] = st[12];
         // row 1 (shift left by 1)
         tmp[1]  = st[5];  tmp[5]  = st[9];  tmp[9]  = st[13]; tmp[13] = st[1];
         // row 2 (shift left by 2)
         tmp[2]  = st[10]; tmp[6]  = st[14]; tmp[10] = st[2];  tmp[14] = st[6];
         // row 3 (shift left by 3)
         tmp[3]  = st[15]; tmp[7]  = st[3];  tmp[11] = st[7];  tmp[15] = st[11];

         for (int i = 0; i < 16; ++i) st[i] = tmp[i];
     }

     void mixColumns(int st[16]) {
         for (int c = 0; c < 4; ++c) {
             int a0 = st[4 * c];
             int a1 = st[4 * c + 1];
             int a2 = st[4 * c + 2];
             int a3 = st[4 * c + 3];

             int b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
             int b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
             int b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
             int b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);

             st[4 * c]     = b0;
             st[4 * c + 1] = b1;
             st[4 * c + 2] = b2;
             st[4 * c + 3] = b3;
         }
     }

     // -----------------------------------------------------------------
     // Helper routines for key schedule
     // -----------------------------------------------------------------
     void rotWord(int w[4]) {
         int t = w[0];
         w[0] = w[1];
         w[1] = w[2];
         w[2] = w[3];
         w[3] = t;
     }

     void subWord(int w[4]) {
         for (int i = 0; i < 4; ++i) w[i] = sBox[w[i]];
     }

     // -----------------------------------------------------------------
     // Finite‑field multiplication helpers (only 2 and 3 are needed)
     // -----------------------------------------------------------------
     int mul2(int x) {
         int r = x << 1;
         if (r & 0x100) r ^= 0x11b;
         return r & 0xFF;
     }

     int mul3(int x) { return mul2(x) ^ x; }
 };

 // ---------------------------------------------------------------------
 // Static tables definition (no const, no unsigned)
 // ---------------------------------------------------------------------
 int Aes256::sBox[256] = {
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

 int Aes256::rCon[15] = {
     0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8,0xAB,0x4D,0x9A
 };

 // ---------------------------------------------------------------------
 // Demo – deterministic test vector (AES‑256 Known Answer Test)
 // ---------------------------------------------------------------------
 int main() {
     // 256‑bit key (32 bytes) – reversed order for adversarial pattern
     int key[32] = {
         0xf4,0xdf,0x14,0x09,0xa3,0x10,0x98,0x2d,
         0xd7,0x08,0x61,0x3b,0x07,0x2c,0x35,0x1f,
         0x81,0x77,0x7d,0x85,0xf0,0xae,0x73,0x2b,
         0xbe,0x71,0xca,0x15,0x10,0xeb,0x3d,0x60
     };

     // 128‑bit plaintext (16 bytes) – reversed order
     int plain[16] = {
         0x2a,0x17,0x93,0x73,
         0x11,0x7e,0x3d,0xe9,
         0x96,0x9f,0x40,0x2e,
         0xe2,0xbe,0xc1,0x6b
     };

     Aes256 engine;
     engine.encrypt(plain, key);

     std::cout << "Ciphertext:" << std::hex << std::setfill('0');
     for (int i = 0; i < 16; ++i) {
         std::cout << std::setw(2) << (plain[i] & 0xFF);
         if (i % 4 == 3) std::cout << ' ';
     }
     std::cout << std::dec << std::endl;
     return 0;
 }
