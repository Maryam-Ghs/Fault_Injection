#include <iostream>
#include <vector>
#include <cstdio>

using namespace std;

/* LLM input variant 1: minimal-boundary */

/*  Anubis‑like block cipher – demonstration version
 *  Uses only int, vector and no const/unsigned/long/double.
 *  Input is generated inside main, result is printed as hex.
 */

class AnubisCipher {
public:
    vector<int> sbox;                 // substitution box
    vector<int> rcon;                 // round constants
    vector< vector<int> > roundKey;   // round keys (including pre‑round)

    // constructor builds S‑box, constants and expands the key
    AnubisCipher(vector<int> key) {
        make_sbox();
        make_rcon();
        expand_key(key);
    }

    // encrypt a 16‑byte block
    void encrypt_block(vector<int> plain, vector<int>& out) {
        vector<int> state = plain;
        add_round_key(state, roundKey[0]);               // initial key addition

        int r = (int)roundKey.size() - 1;                // number of rounds
        int i = 1;
        while (i < r) {                                 // all but last round
            sub_bytes(state);
            shift_rows(state);
            mix_columns(state);
            add_round_key(state, roundKey[i]);
            ++i;
        }
        sub_bytes(state);
        shift_rows(state);
        add_round_key(state, roundKey[r]);               // final key addition
        out = state;
    }

private:
    // --------------------------------------------------------------------
    //  Helper transformations
    // --------------------------------------------------------------------
    void make_sbox() {
        sbox = {
            0xa8,0x43,0x5f,0x06,0x6b,0x75,0x6c,0x59,0x71,0xdf,0x87,0x95,0x17,0xf0,0xd8,0x09,
            0x6a,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,
            0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,0x1f,0xdd,0xa2,0x33,0x88,0x07,0xc7,0x31,
            0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5d,0x0c,0x7d,0x7b,0x72,0x00,0x5c,0x44,0x81,
            0x08,0x4c,0x0a,0x2e,0x91,0x6f,0xb6,0x3f,0x8c,0x73,0x19,0x2d,0x6e,0x84,0x9e,0x95,
            0x48,0x45,0x4f,0x4a,0x9b,0x2c,0x44,0x57,0x9c,0x15,0x63,0x8c,0x79,0x2b,0x5e,0xf9,
            0x0d,0x6d,0x0f,0x0a,0x8e,0x84,0xa5,0x0c,0x06,0xd3,0x2c,0x42,0x41,0xc2,0x11,0x2f,
            0xf2,0xdb,0x65,0x6a,0x1c,0x6c,0x5b,0x6a,0x0b,0x86,0x1f,0x5d,0x6e,0x4f,0x2e,0x63,
            0x3c,0x1d,0x5a,0x57,0xd5,0x9f,0x5d,0x70,0x49,0x6c,0x2b,0xbd,0x2f,0x50,0x7f,0x6c,
            0x1b,0x31,0x8c,0x2f,0x80,0x0e,0x5a,0x0c,0xf9,0x04,0x0e,0x6c,0x63,0x5c,0x2a,0x78,
            0x57,0x45,0x2c,0x6e,0x4b,0x74,0x2c,0x5f,0x3e,0x7c,0x62,0x4c,0x9c,0x93,0x2b,0x3a,
            0x6b,0x8b,0x8b,0x6b,0x31,0x74,0x8c,0x91,0x69,0x5d,0x8c,0x9f,0x88,0x3b,0x1b,0xc2,
            0x8c,0x0e,0x3e,0xa5,0x3d,0x2c,0x14,0x2a,0x2d,0x4f,0x9b,0x5f,0x0f,0x3a,0x4c,0x2c,
            0x88,0x5d,0x6e,0x2c,0x1f,0x4c,0x9e,0x4c,0x0c,0x8d,0x40,0x7d,0x0c,0x6b,0x1d,0x02,
            0x1e,0x1a,0x70,0x37,0x0c,0x9e,0x0b,0x9c,0x9d,0x26,0x8e,0x6b,0x11,0x5b,0x21,0x0d,
            0x2b,0x5f,0x71,0x00,0x1b,0x5f,0x00,0x70,0x33,0x77,0x5f,0x00,0x5c,0x00,0x5c,0x00
        };
    }

    void make_rcon() {
        rcon = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8 };
    }

    // simple (non‑standard) key schedule – enough for a demo
    void expand_key(vector<int> key) {
        roundKey.clear();
        roundKey.push_back(key);                     // round 0 key
        int round = 0;
        while (round < 12) {
            int j = 0;
            while (j < 16) {
                key[j] = sbox[key[j] ^ rcon[round]];
                ++j;
            }
            roundKey.push_back(key);
            ++round;
        }
    }

    // --------------------------------------------------------------------
    //  Core round functions
    // --------------------------------------------------------------------
    void sub_bytes(vector<int>& st) {
        int i = 0;
        while (i < 16) {
            st[i] = sbox[st[i]];
            ++i;
        }
    }

    void shift_rows(vector<int>& st) {
        vector<int> tmp(16);
        int row = 0;
        while (row < 4) {
            int col = 0;
            while (col < 4) {
                // matrix is column‑major; rotate each row left by its index
                tmp[row + 4*col] = st[row + 4*((col + row) % 4)];
                ++col;
            }
            ++row;
        }
        st = tmp;
    }

    // very light‑weight linear mixing – not the real Anubis MDS
    void mix_columns(vector<int>& st) {
        int col = 0;
        while (col < 4) {
            int a0 = st[4*col];
            int a1 = st[4*col + 1];
            int a2 = st[4*col + 2];
            int a3 = st[4*col + 3];
            st[4*col]     = a0 ^ a1;
            st[4*col + 1] = a1 ^ a2;
            st[4*col + 2] = a2 ^ a3;
            st[4*col + 3] = a3 ^ a0;
            ++col;
        }
    }

    void add_round_key(vector<int>& st, const vector<int>& rk) {
        int k = 0;
        while (k < 16) {
            st[k] ^= rk[k];
            ++k;
        }
    }
};

int main() {
    // ---- generate a 128‑bit key (16 ints) with minimal boundary values ----
    vector<int> key(16);
    int p = 0;
    while (p < 16) {
        key[p] = 0;   // minimal byte value
        ++p;
    }

    // ---- generate a 128‑bit plaintext block with minimal boundary values ----
    vector<int> plain(16);
    p = 0;
    while (p < 16) {
        plain[p] = 0;   // minimal byte value
        ++p;
    }

    // ---- encrypt ----
    AnubisCipher cipher(key);
    vector<int> encrypted;
    cipher.encrypt_block(plain, encrypted);

    // ---- print ciphertext as hex ----
    p = 0;
    while (p < 16) {
        printf("%02x", encrypted[p] & 0xFF);
        if (p % 4 == 3) printf(" ");
        ++p;
    }
    printf("\n");
    return 0;
}
