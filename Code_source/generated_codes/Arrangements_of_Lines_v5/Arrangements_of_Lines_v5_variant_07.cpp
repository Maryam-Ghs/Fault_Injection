#include <cstdio>

/* LLM input variant 7: reverse-adversarial */

class Line {
public:
    int A, B, C;
};

class Arrangement {
    int N;
    Line L[10];
    int I;
public:
    Arrangement() : N(6), I(0) { gen(); }
    
    void gen() {
        // Reverse ordered B values to create worst‑case ordering for some algorithms
        for (int k = 0; k < N; ++k) {
            L[k].A = 1;
            L[k].B = N - 1 - k;          // 5,4,3,2,1,0 (reverse)
            L[k].C = k * 3;
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
