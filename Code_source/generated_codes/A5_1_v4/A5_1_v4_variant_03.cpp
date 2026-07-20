#include <iostream>
#include <cstdlib>
#include <ctime>

#define R1LEN 19
#define R2LEN 22
#define R3LEN 23
#define KEYLEN 64
#define FRMLEN 22
#define STREAMLEN 114   // length of one GSM burst

/* LLM input variant 3: zeros-and-ones */

/* ---------- helper functions ----------------------------------- */
int majority(int x, int y, int z) {
    return (x + y + z) >= 2 ? 1 : 0;
}

// XOR of the tapped bits of a register
int feedback(int reg[], int taps[], int tcnt) {
    int fb = 0;
    for (int i = 0; i < tcnt; ++i) fb ^= reg[taps[i]];
    return fb;
}

// shift right, inserting newbit at position 0
void shift(int reg[], int sz, int newbit) {
    for (int i = sz - 1; i > 0; --i) reg[i] = reg[i - 1];
    reg[0] = newbit;
}

/* ---------- A5/1 core ------------------------------------------ */
void initialise(int r1[], int r2[], int r3[], int key[], int frm[]) {
    // load the 64‑bit key
    for (int i = 0; i < KEYLEN; ++i) {
        int fb1 = feedback(r1, tap1, 4) ^ key[i];
        int fb2 = feedback(r2, tap2, 2) ^ key[i];
        int fb3 = feedback(r3, tap3, 4) ^ key[i];
        shift(r1, R1LEN, fb1);
        shift(r2, R2LEN, fb2);
        shift(r3, R3LEN, fb3);
    }

    // load the 22‑bit frame number
    for (int i = 0; i < FRMLEN; ++i) {
        int fb1 = feedback(r1, tap1, 4) ^ frm[i];
        int fb2 = feedback(r2, tap2, 2) ^ frm[i];
        int fb3 = feedback(r3, tap3, 4) ^ frm[i];
        shift(r1, R1LEN, fb1);
        shift(r2, R2LEN, fb2);
        shift(r3, R3LEN, fb3);
    }

    // 100 warm‑up cycles with majority clocking
    int cnt = 0;
    while (cnt < 100) {
        int maj = majority(r1[8], r2[10], r3[10]);

        if (r1[8] == maj) {
            int fb = feedback(r1, tap1, 4);
            shift(r1, R1LEN, fb);
        }
        if (r2[10] == maj) {
            int fb = feedback(r2, tap2, 2);
            shift(r2, R2LEN, fb);
        }
        if (r3[10] == maj) {
            int fb = feedback(r3, tap3, 4);
            shift(r3, R3LEN, fb);
        }
        ++cnt;
    }
}

void generate_keystream(int r1[], int r2[], int r3[], int out[]) {
    int pos = 0;
    while (pos < STREAMLEN) {
        // keystream bit = XOR of the three output bits
        out[pos] = r1[R1LEN - 1] ^ r2[R2LEN - 1] ^ r3[R3LEN - 1];

        // majority‑driven clocking for the next step
        int maj = majority(r1[8], r2[10], r3[10]);

        if (r1[8] == maj) {
            int fb = feedback(r1, tap1, 4);
            shift(r1, R1LEN, fb);
        }
        if (r2[10] == maj) {
            int fb = feedback(r2, tap2, 2);
            shift(r2, R2LEN, fb);
        }
        if (r3[10] == maj) {
            int fb = feedback(r3, tap3, 4);
            shift(r3, R3LEN, fb);
        }
        ++pos;
    }
}

/* ---------- main ------------------------------------------------ */
int main() {
    // stack‑allocated registers
    int reg1[R1LEN] = {0};
    int reg2[R2LEN] = {0};
    int reg3[R3LEN] = {0};

    // deterministic key (alternating 0 and 1) and frame (all zeros)
    int key[KEYLEN];
    int frame[FRMLEN];

    for (int i = 0; i < KEYLEN; ++i) key[i] = i % 2;   // 0,1,0,1,...
    for (int i = 0; i < FRMLEN; ++i) frame[i] = 0;    // all zeros

    initialise(reg1, reg2, reg3, key, frame);

    int stream[STREAMLEN];
    generate_keystream(reg1, reg2, reg3, stream);

    // print the keystream as a binary string
    std::cout << "A5/1 keystream (" << STREAMLEN << " bits):\n";
    for (int i = 0; i < STREAMLEN; ++i) std::cout << stream[i];
    std::cout << std::endl;

    return 0;
}
