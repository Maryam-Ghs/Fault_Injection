#include <cstdio>

/* LLM input variant 10: large-safe-stress */

class Line {
public:
    int A, B, C;
};

class Arrangement {
    int N;
    Line *L;            // dynamically allocated for large N
    int I;
public:
    Arrangement() : N(1000), I(0) {
        L = new Line[N];
        gen();
    }
    
    ~Arrangement() {
        delete[] L;
    }
    
    void gen() {
        // Generate a large but safe set of lines with varied parameters
        for (int k = 0; k < N; ++k) {
            L[k].A = 1 + (k % 3);                // 1,2,3 repeating
            L[k].B = k;                          // distinct slopes
            L[k].C = (k * 7) % 1000 - 500;       // values in roughly [-500,499]
        }
    }
    
    void calc() {
        for (int p = 0; p < N; ++p) {
            for (int q = p + 1; q < N; ++q) {
                I += (L[p].A * L[q].B != L[p].B * L[q].A);
            }
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
