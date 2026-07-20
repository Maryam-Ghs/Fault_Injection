#include <cstdio>

/* LLM input variant 3: zeros-and-ones */

class Line {
public:
    int A, B, C;
};

class Arrangement {
    int N;
    Line L[10];
    int I;
public:
    Arrangement() : N(3), I(0) { gen(); }
    
    void gen() {
        // Emphasize zeros and ones
        L[0].A = 0;  L[0].B = 0;  L[0].C = 0;
        L[1].A = 1;  L[1].B = 1;  L[1].C = 1;
        L[2].A = 1;  L[2].B = 0;  L[2].C = 1;
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
