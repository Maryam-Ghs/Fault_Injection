#include <cstdio>

/* LLM input variant 6: ordered-structured */

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
        // Sorted B values, symmetric C values
        for (int k = 0; k < N; ++k) {
            L[k].A = 1;
            L[k].B = k;                 // 0,1,2,3,4,5 (ordered)
            L[k].C = (k % 2 == 0) ? k : -k; // 0,-1,2,-3,4,-5
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
