/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <iomanip>

// ------------------------------------------------------------
// AES-128 ECB implementation (Version #5)
// ------------------------------------------------------------

// ------------------------------------------------------------
// Helper: multiplication by x in GF(2^8)
// ------------------------------------------------------------
int xtime_step(int val) {
    int shifted = (val << 1) & 0xFF;
    int mask    = (val & 0x80) ? 0x1B : 0x00;
    int result  = shifted ^ mask;
    return result & 0xFF;
}

// ------------------------------------------------------------
// S‑Box (filled at runtime, but stored in a heap array)
// ------------------------------------------------------------
int* build_sbox() {
    int* box = new int[256];
    // Standard AES S‑Box values
    int raw[256] = {
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
    for (int i = 0; i < 256; ++i) {
        box[i] = raw[i];
    }
    return box;
}

// ------------------------------------------------------------
// Rcon (round constants)
// ------------------------------------------------------------
int* build_rcon() {
    int* rc = new int[11];
    rc[0] = 0x00;
    rc[1] = 0x01;
    rc[2] = 0x02;
    rc[3] = 0x04;
    rc[4] = 0x08;
    rc[5] = 0x10;
    rc[6] = 0x20;
    rc[7] = 0x40;
    rc[8] = 0x80;
    rc[9] = 0x1B;
    rc[10]= 0x36;
    return rc;
}

// ------------------------------------------------------------
// Key Expansion (produces 44 words = 176 bytes)
// ------------------------------------------------------------
int* expand_key(const int* masterKey, const int* sbox, const int* rcon) {
    int* schedule = new int[176];               // 44 * 4 bytes
    // Copy the original 16‑byte key
    for (int i = 0; i < 16; ++i) {
        schedule[i] = masterKey[i];
    }

    int bytesGenerated = 16;                    // already have 4 words
    int rconIter = 1;                           // Rcon index

    while (bytesGenerated < 176) {
        // ---- 1. Temporary word (last 4 bytes) ----
        int temp0 = schedule[bytesGenerated - 4];
        int temp1 = schedule[bytesGenerated - 3];
        int temp2 = schedule[bytesGenerated - 2];
        int temp3 = schedule[bytesGenerated - 1];

        // ---- 2. Rotate word every 16 bytes ----
        if ((bytesGenerated % 16) == 0) {
            // Rotate left
            int rot0 = temp1;
            int rot1 = temp2;
            int rot2 = temp3;
            int rot3 = temp0;

            // Apply S‑Box
            int sub0 = sbox[rot0];
            int sub1 = sbox[rot1];
            int sub2 = sbox[rot2];
            int sub3 = sbox[rot3];

            // XOR with Rcon
            int rconVal = rcon[rconIter];
            temp0 = sub0 ^ rconVal;
            temp1 = sub1;
            temp2 = sub2;
            temp3 = sub3;

            rconIter += 1;
        }
        else if ((bytesGenerated % 16) == 4) {
            // Only SubBytes, no rotation or Rcon
            temp0 = sbox[temp0];
            temp1 = sbox[temp1];
            temp2 = sbox[temp2];
            temp3 = sbox[temp3];
        }

        // ---- 3. XOR with word 16 bytes earlier ----
        int prev0 = schedule[bytesGenerated - 16];
        int prev1 = schedule[bytesGenerated - 15];
        int prev2 = schedule[bytesGenerated - 14];
        int prev3 = schedule[bytesGenerated - 13];

        schedule[bytesGenerated]     = prev0 ^ temp0;
        schedule[bytesGenerated + 1] = prev1 ^ temp1;
        schedule[bytesGenerated + 2] = prev2 ^ temp2;
        schedule[bytesGenerated + 3] = prev3 ^ temp3;

        bytesGenerated += 4;
    }
    return schedule;
}

// ------------------------------------------------------------
// AddRoundKey
// ------------------------------------------------------------
void apply_round_key(int* state, const int* roundKey, int startIdx) {
    for (int i = 0; i < 16; ++i) {
        state[i] ^= roundKey[startIdx + i];
    }
}

// ------------------------------------------------------------
// SubBytes
// ------------------------------------------------------------
void substitute_bytes(int* state, const int* sbox) {
    for (int i = 0; i < 16; ++i) {
        state[i] = sbox[state[i]];
    }
}

// ------------------------------------------------------------
// ShiftRows (explicit per row)
// ------------------------------------------------------------
void shift_rows(int* state) {
    // Row 1 (index 1,5,9,13) shift left by 1
    int a1 = state[1];
    int a5 = state[5];
    int a9 = state[9];
    int a13= state[13];
    state[1]  = a5;
    state[5]  = a9;
    state[9]  = a13;
    state[13] = a1;

    // Row 2 (index 2,6,10,14) shift left by 2
    int b2 = state[2];
    int b6 = state[6];
    int b10= state[10];
    int b14= state[14];
    state[2]  = b10;
    state[6]  = b14;
    state[10] = b2;
    state[14] = b6;

    // Row 3 (index 3,7,11,15) shift left by 3 (right by 1)
    int c3 = state[3];
    int c7 = state[7];
    int c11= state[11];
    int c15= state[15];
    state[3]  = c15;
    state[7]  = c3;
    state[11] = c7;
    state[15] = c11;
}

// ------------------------------------------------------------
// MixColumns (expanded, step‑by‑step)
// ------------------------------------------------------------
void mix_columns(int* state) {
    for (int col = 0; col < 4; ++col) {
        int idx0 = col * 4;
        int idx1 = idx0 + 1;
        int idx2 = idx0 + 2;
        int idx3 = idx0 + 3;

        int s0 = state[idx0];
        int s1 = state[idx1];
        int s2 = state[idx2];
        int s3 = state[idx3];

        // Multiply by 2
        int m2_0 = xtime_step(s0);
        int m2_1 = xtime_step(s1);
        int m2_2 = xtime_step(s2);
        int m2_3 = xtime_step(s3);

        // Multiply by 3 = xtime(x) ^ x
        int m3_0 = m2_0 ^ s0;
        int m3_1 = m2_1 ^ s1;
        int m3_2 = m2_2 ^ s2;
        int m3_3 = m2_3 ^ s3;

        // Compute new column values
        int r0 = m2_0 ^ m3_1 ^ s2   ^ s3;
        int r1 = s0   ^ m2_1 ^ m3_2 ^ s3;
        int r2 = s0   ^ s1   ^ m2_2 ^ m3_3;
        int r3 = m3_0 ^ s1   ^ s2   ^ m2_3;

        state[idx0] = r0 & 0xFF;
        state[idx1] = r1 & 0xFF;
        state[idx2] = r2 & 0xFF;
        state[idx3] = r3 & 0xFF;
    }
}

// ------------------------------------------------------------
// Encrypt a single 16‑byte block
// ------------------------------------------------------------
void encrypt_block(int* block, const int* roundKeys, const int* sbox) {
    // Initial AddRoundKey
    apply_round_key(block, roundKeys, 0);

    // 9 main rounds
    int roundStart = 16;
    int round = 1;
    while (round < 10) {
        substitute_bytes(block, sbox);
        shift_rows(block);
        mix_columns(block);
        apply_round_key(block, roundKeys, roundStart);
        roundStart += 16;
        round += 1;
    }

    // Final round (no MixColumns)
    substitute_bytes(block, sbox);
    shift_rows(block);
    apply_round_key(block, roundKeys, roundStart);
}

// ------------------------------------------------------------
// Helper: print a 16‑byte array as hex
// ------------------------------------------------------------
void show_hex(const int* data) {
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (data[i] & 0xFF);
        if (i != 15) std::cout << " ";
    }
    std::cout << std::dec << std::endl;
}

// ------------------------------------------------------------
// Main – generate data, run AES, display result
// ------------------------------------------------------------
int main() {
    // ---- 1. Define a reverse‑ordered adversarial plaintext (16 bytes) ----
    int* plain = new int[16];
    int sample[16] = {
        0xFF,0xFE,0xFD,0xFC,
        0xFB,0xFA,0xF9,0xF8,
        0xF7,0xF6,0xF5,0xF4,
        0xF3,0xF2,0xF1,0xF0
    };
    for (int i = 0; i < 16; ++i) plain[i] = sample[i];

    // ---- 2. Define a reverse‑ordered adversarial 128‑bit key (16 bytes) ----
    int* key = new int[16];
    int keyBytes[16] = {
        0xFF,0xEE,0xDD,0xCC,
        0xBB,0xAA,0x99,0x88,
        0x77,0x66,0x55,0x44,
        0x33,0x22,0x11,0x00
    };
    for (int i = 0; i < 16; ++i) key[i] = keyBytes[i];

    // ---- 3. Build S‑Box and Rcon tables ----
    int* sbox = build_sbox();
    int* rcon = build_rcon();

    // ---- 4. Expand the key ----
    int* roundKeys = expand_key(key, sbox, rcon);

    // ---- 5. Encrypt the block ----
    encrypt_block(plain, roundKeys, sbox);

    // ---- 6. Output the ciphertext ----
    std::cout << "Ciphertext (hex): ";
    show_hex(plain);

    // ---- 7. Clean up heap memory ----
    delete[] plain;
    delete[] key;
    delete[] sbox;
    delete[] rcon;
    delete[] roundKeys;

    return 0;
}
