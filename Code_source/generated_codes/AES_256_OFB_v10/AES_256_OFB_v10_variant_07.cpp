/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

class AES256OFB {
public:
    AES256OFB() {
        roundKeyArray = new int[240];          // 15 round keys * 16 bytes
        ivArray       = new int[16];           // Initialization vector
        // Build multiplication tables for GF(2^8)
        buildMulTables();
    }

    ~AES256OFB() {
        delete[] roundKeyArray;
        delete[] ivArray;
    }

    /*-------------------------------------------------------------
     *  Set the 256‑bit key (32 bytes)
     *------------------------------------------------------------*/
    void setKey(int* keyBytes) {
        keyExpansion(keyBytes);
    }

    /*-------------------------------------------------------------
     *  Set the 128‑bit IV (16 bytes)
     *------------------------------------------------------------*/
    void setIV(int* ivBytes) {
        int i = 0;
        while (i < 16) {
            ivArray[i] = ivBytes[i];
            i = i + 1;
        }
    }

    /*-------------------------------------------------------------
     *  OFB encryption: ciphertext = plaintext XOR keystream
     *------------------------------------------------------------*/
    void ofbEncrypt(int* plainBytes, int dataLen, int* cipherBytes) {
        // Working buffer for the current keystream block
        int* keystream = new int[16];
        int* nextBlock = new int[16];
        int i = 0;

        // Initialise keystream with the IV
        while (i < 16) {
            keystream[i] = ivArray[i];
            i = i + 1;
        }

        int offset = 0;
        while (offset < dataLen) {
            // Generate next keystream block = AES(keystream)
            encryptBlock(keystream, nextBlock);

            // XOR plaintext with keystream to produce ciphertext
            int j = 0;
            while (j < 16 && (offset + j) < dataLen) {
                cipherBytes[offset + j] = plainBytes[offset + j] ^ nextBlock[j];
                j = j + 1;
            }

            // Prepare for the next iteration
            int k = 0;
            while (k < 16) {
                keystream[k] = nextBlock[k];
                k = k + 1;
            }

            offset = offset + 16;
        }

        delete[] keystream;
        delete[] nextBlock;
    }

private:
    int* roundKeyArray;    // 240 ints = 15 * 16
    int* ivArray;          // 16 ints
    int  mul2[256];
    int  mul3[256];

    /*-------------------------------------------------------------
     *  Build multiplication tables for 2·x and 3·x in GF(2^8)
     *------------------------------------------------------------*/
    void buildMulTables() {
        int i = 0;
        while (i < 256) {
            int x = i;
            int xt = x << 1;
            if (x & 0x80) {
                xt = xt ^ 0x1b;
            }
            mul2[i] = xt & 0xff;
            mul3[i] = (mul2[i] ^ x) & 0xff;
            i = i + 1;
        }
    }

    /*-------------------------------------------------------------
     *  S‑Box (static, 256 entries)
     *------------------------------------------------------------*/
    static int sbox[256];

    /*-------------------------------------------------------------
     *  Rcon array for key expansion (15 entries, first is 0x01)
     *------------------------------------------------------------*/
    static int rcon[15];

    /*-------------------------------------------------------------
     *  Key expansion for AES‑256 (produces 15 round keys)
     *------------------------------------------------------------*/
    void keyExpansion(int* keyBytes) {
        // Copy original 256‑bit key into the first 32 bytes of roundKeyArray
        int i = 0;
        while (i < 32) {
            roundKeyArray[i] = keyBytes[i];
            i = i + 1;
        }

        // Generate remaining words (total 60 words = 240 bytes)
        int wordIndex = 8;          // Already have 8 words (32 bytes)
        while (wordIndex < 60) {
            // Temporary word = previous word
            int temp[4];
            int t = 0;
            while (t < 4) {
                temp[t] = roundKeyArray[(wordIndex - 1) * 4 + t];
                t = t + 1;
            }

            // Apply core schedule every 8th word
            if ((wordIndex % 8) == 0) {
                // Rotate left by one byte
                int rot = temp[0];
                int r = 0;
                while (r < 3) {
                    temp[r] = temp[r + 1];
                    r = r + 1;
                }
                temp[3] = rot;

                // SubBytes on the rotated word
                int s = 0;
                while (s < 4) {
                    temp[s] = sbox[temp[s]];
                    s = s + 1;
                }

                // XOR first byte with Rcon
                temp[0] = (temp[0] ^ rcon[wordIndex / 8]) & 0xff;
            }
            else if ((wordIndex % 8) == 4) {
                // SubBytes on the word
                int s = 0;
                while (s < 4) {
                    temp[s] = sbox[temp[s]];
                    s = s + 1;
                }
            }

            // XOR with word 8 positions earlier
            int j = 0;
            while (j < 4) {
                roundKeyArray[wordIndex * 4 + j] =
                    (roundKeyArray[(wordIndex - 8) * 4 + j] ^ temp[j]) & 0xff;
                j = j + 1;
            }

            wordIndex = wordIndex + 1;
        }
    }

    /*-------------------------------------------------------------
     *  AddRoundKey – XOR state with round key
     *------------------------------------------------------------*/
    void addRoundKey(int* state, int round) {
        int i = 0;
        while (i < 16) {
            state[i] = (state[i] ^ roundKeyArray[round * 16 + i]) & 0xff;
            i = i + 1;
        }
    }

    /*-------------------------------------------------------------
     *  SubBytes – apply S‑Box to each byte of the state
     *------------------------------------------------------------*/
    void subBytes(int* state) {
        int i = 0;
        while (i < 16) {
            state[i] = sbox[state[i]];
            i = i + 1;
        }
    }

    /*-------------------------------------------------------------
     *  ShiftRows – cyclic shift of rows in the state matrix
     *------------------------------------------------------------*/
    void shiftRows(int* state) {
        // Row 0: no shift (indices 0,4,8,12)
        // Row 1: shift left by 1 (indices 1,5,9,13)
        // Row 2: shift left by 2 (indices 2,6,10,14)
        // Row 3: shift left by 3 (indices 3,7,11,15)

        // Perform explicit swaps for clarity
        int tmp;

        // Row 1
        tmp = state[1];
        state[1] = state[5];
        state[5] = state[9];
        state[9] = state[13];
        state[13] = tmp;

        // Row 2 (swap pairs)
        tmp = state[2];
        state[2] = state[10];
        state[10] = tmp;
        tmp = state[6];
        state[6] = state[14];
        state[14] = tmp;

        // Row 3
        tmp = state[15];
        state[15] = state[11];
        state[11] = state[7];
        state[7] = state[3];
        state[3] = tmp;
    }

    /*-------------------------------------------------------------
     *  MixColumns – matrix multiplication in GF(2^8)
     *------------------------------------------------------------*/
    void mixColumns(int* state) {
        int col = 0;
        while (col < 4) {
            int idx = col * 4;
            int s0 = state[idx];
            int s1 = state[idx + 1];
            int s2 = state[idx + 2];
            int s3 = state[idx + 3];

            int m0 = (mul2[s0] ^ mul3[s1] ^ s2 ^ s3) & 0xff;
            int m1 = (s0 ^ mul2[s1] ^ mul3[s2] ^ s3) & 0xff;
            int m2 = (s0 ^ s1 ^ mul2[s2] ^ mul3[s3]) & 0xff;
            int m3 = (mul3[s0] ^ s1 ^ s2 ^ mul2[s3]) & 0xff;

            state[idx]     = m0;
            state[idx + 1] = m1;
            state[idx + 2] = m2;
            state[idx + 3] = m3;

            col = col + 1;
        }
    }

    /*-------------------------------------------------------------
     *  Encrypt a single 16‑byte block (ECB mode, used by OFB)
     *------------------------------------------------------------*/
    void encryptBlock(int* inBlock, int* outBlock) {
        // Copy input to state array
        int i = 0;
        while (i < 16) {
            outBlock[i] = inBlock[i] & 0xff;
            i = i + 1;
        }

        // Initial round key addition
        addRoundKey(outBlock, 0);

        // Rounds 1 … 13
        int round = 1;
        while (round <= 13) {
            subBytes(outBlock);
            shiftRows(outBlock);
            mixColumns(outBlock);
            addRoundKey(outBlock, round);
            round = round + 1;
        }

        // Final round (no MixColumns)
        subBytes(outBlock);
        shiftRows(outBlock);
        addRoundKey(outBlock, 14);
    }
};

/*-------------------------------------------------------------
 *  Static data definitions (S‑Box and Rcon)
 *------------------------------------------------------------*/
int AES256OFB::sbox[256] = {
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

int AES256OFB::rcon[15] = {
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
    0x6C,0xD8,0xAB,0x4D,0x9A
};

/*-------------------------------------------------------------
 *  Main function – generate deterministic reverse‑ordered data,
 *  encrypt, and display
 *------------------------------------------------------------*/
int main() {
    // ------------------ Deterministic descending 256‑bit key ------------------
    int* keyBytes = new int[32];
    int i = 0;
    while (i < 32) {
        keyBytes[i] = 255 - i;   // 255,254,...,224
        i = i + 1;
    }

    // ------------------ Deterministic descending 128‑bit IV -------------------
    int* ivBytes = new int[16];
    i = 0;
    while (i < 16) {
        ivBytes[i] = 255 - i;   // 255,254,...,240
        i = i + 1;
    }

    // ------------------ Deterministic descending plaintext (63 bytes) -------
    int dataLength = 63;   // Not a multiple of 16 to test partial block handling
    int* plainBytes = new int[dataLength];
    i = 0;
    while (i < dataLength) {
        plainBytes[i] = 255 - i;   // 255,254,... descending
        i = i + 1;
    }

    // Allocate buffer for ciphertext
    int* cipherBytes = new int[dataLength];

    // ------------------ Initialise AES engine -----------------------
    AES256OFB aesEngine;
    aesEngine.setKey(keyBytes);
    aesEngine.setIV(ivBytes);

    // ------------------ Perform OFB encryption ----------------------
    aesEngine.ofbEncrypt(plainBytes, dataLength, cipherBytes);

    // ------------------ Display results -----------------------------
    std::cout << "Key (256‑bit) : ";
    i = 0;
    while (i < 32) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (plainBytes[i] & 0xff) << " ";
        i = i + 1;
    }
    std::cout << std::dec << "\n";

    std::cout << "IV (128‑bit)  : ";
    i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (ivBytes[i] & 0xff) << " ";
        i = i + 1;
    }
    std::cout << std::dec << "\n";

    std::cout << "Plaintext     : ";
    i = 0;
    while (i < dataLength) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (plainBytes[i] & 0xff) << " ";
        i = i + 1;
    }
    std::cout << std::dec << "\n";

    std::cout << "Ciphertext    : ";
    i = 0;
    while (i < dataLength) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipherBytes[i] & 0xff) << " ";
        i = i + 1;
    }
    std::cout << std::dec << "\n";

    // ------------------ Clean up ------------------------------------
    delete[] keyBytes;
    delete[] ivBytes;
    delete[] plainBytes;
    delete[] cipherBytes;

    return 0;
}
