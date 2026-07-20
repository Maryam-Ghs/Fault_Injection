#include <cstdio>

/* LLM input variant 8: sparse-skewed */

class Line {
public:
    int A, B, C;
};

class Arrangement {
    int N;
    Line L[10];
    int I;
public:
    Arrangement() : N(7), I(0) { gen(); }
    
    void gen() {
        // Mostly zero slopes, a few outliers
        L[0].A = 1; L[0].B = 0;  L[0].C = 5;
        L[1].A = 1; L[1].B = 0;  L[1].C = 8;
        L[2].A = 1; L[2].B = 0;  L[2].C = 2;
        L[3].A = 1; L[3].B = 3;  L[3].C = 1;  // outlier
        L[4].A = 1; L[4].B = 0;  L[4].C = 7;
        L[5].A = 1; L[5].B = -2; L[5].C = 4;  // negative slope outlier
        L[6].A = 1; L[6].B = 0;  L[6].C = 0;
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
