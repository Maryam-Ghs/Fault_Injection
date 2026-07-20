#include <cstdio>

/* LLM input variant 9: medium-deterministic-random */

class Line {
public:
    int A, B, C;
};

class Arrangement {
    int N;
    Line L[20];
    int I;
public:
    Arrangement() : N(12), I(0) { gen(); }
    
    void gen() {
        // Simple deterministic pseudo‑random generator (linear congruential)
        unsigned int seed = 12345;
        for (int k = 0; k < N; ++k) {
            seed = (1103515245 * seed + 12345) & 0x7fffffff;
            L[k].A = (seed % 5) + 1;               // 1..5
            seed = (1103515245 * seed + 12345) & 0x7fffffff;
            L[k].B = (int)((seed % 21) - 10);      // -10..10
            seed = (1103515245 * seed + 12345) & 0x7fffffff;
            L[k].C = (int)((seed % 31) - 15);      // -15..15
        }
    }
    
    void calc() {
        int p = 0;
        while (p < N) {
            int q = p + 1;
            while (q < N) {
                I += (L[p].A * L[q].B != L[p].B * L[q].A);
                q++;
            }
            p++;
        }
    }
    
    int result() {
        return 1 + N + I;
    }
    
    void show() {
        calc();
        printf("Lines   : %d\n", N);
        printf("Regions : %d\n", result());
    }
};

int main() {
    Arrangement obj;
    obj.show();
    return 0;
}
