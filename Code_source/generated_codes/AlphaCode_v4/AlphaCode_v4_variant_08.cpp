#include <iostream>

/* LLM input variant 8: sparse-skewed */

int main() {
    // Version #4 of AlphaCode implementation
    // Deterministic test vectors (sparse and skewed)
    int testVals[6] = {0, 0, 0, 0, 5, 1500};

    // Process each test vector
    int pos = 0;
    while (pos < 6) {
        // Current input
        int n = testVals[pos];

        // Compute sum of squares: S = 1^2 + 2^2 + ... + n^2
        int i = 1;
        int sumSq = 0;
        while (i <= n) {
            int sq = i * i;          // temporary square
            sumSq = sumSq + sq;      // accumulate
            i = i + 1;               // next i
        }

        // Compute triangular number: T = 1 + 2 + ... + n
        int tri = 0;
        for (int j = 1; j <= n; ++j) {
            int add = j;             // temporary addend
            tri = tri + add;         // accumulate
        }

        // Compute a simple linear combination: L = 2*S - 3*T
        int twoS = sumSq + sumSq;          // 2 * S
        int threeT = tri + tri + tri;      // 3 * T
        int linComb = twoS - threeT;       // L

        // Output the results
        std::cout << "Input n = " << n << "\n";
        std::cout << "  Sum of squares = " << sumSq << "\n";
        std::cout << "  Triangular number = " << tri << "\n";
        std::cout << "  Linear combo (2*S - 3*T) = " << linComb << "\n";
        std::cout << "--------------------------\n";

        // Move to next test vector
        pos = pos + 1;
    }

    return 0;
}
