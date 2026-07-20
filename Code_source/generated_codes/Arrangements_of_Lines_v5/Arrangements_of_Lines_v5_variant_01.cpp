#include <cstdio>

/* LLM input variant 1: minimal-boundary */

class Line {
public:
    int A, B, C;
};

class Arrangement {
    int N;
    Line L[10];
    int I;               // intersection count
public:
    Arrangement() : N(1), I(0) { gen(); }
    
    void gen() {
        // Single line with smallest non‑trivial values
        L[0].A = 1;
        L[0].B = 0;
        L[0].C = 0;
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
        return 1 + N + I;   // regions = 1 + lines + intersections
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
