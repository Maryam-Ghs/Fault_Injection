#include <cstdio>

/* LLM input variant 2: small-diverse */

class Line {
public:
    int A, B, C;
};

class Arrangement {
    int N;
    Line L[10];
    int I;               
public:
    Arrangement() : N(4), I(0) { gen(); }
    
    void gen() {
        // Varied ordinary values
        L[0].A = 2;  L[0].B = 0;  L[0].C = 5;
        L[1].A = 3;  L[1].B = 2;  L[1].C = -1;
        L[2].A = 1;  L[2].B = 5;  L[2].C = 10;
        L[3].A = 4;  L[3].B = 7;  L[3].C = 0;
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
